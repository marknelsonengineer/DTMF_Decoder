///////////////////////////////////////////////////////////////////////////////
//          University of Hawaii, College of Engineering
//          DTMF_Decoder - EE 469 - Fall 2022
//
//  A Windows Desktop C program that decodes DTMF tones
//
/// Windows Audio Driver code
/// 
/// @file audio.cpp
/// @version 1.0
///
/// @see https://learn.microsoft.com/en-us/windows/win32/api/_coreaudio/
/// @see https://learn.microsoft.com/en-us/windows/win32/api/mmdeviceapi/nn-mmdeviceapi-immdevice
/// @see https://learn.microsoft.com/en-us/windows/win32/api/audioclient/nn-audioclient-iaudioclient
/// @see https://learn.microsoft.com/en-us/windows/win32/api/audioclient/nf-audioclient-iaudioclient-initialize
/// @see https://learn.microsoft.com/en-us/windows/win32/api/audioclient/nf-audioclient-iaudioclient-getbuffersize
/// @see https://learn.microsoft.com/en-us/windows/win32/api/propsys/nn-propsys-ipropertystore
/// @see https://learn.microsoft.com/en-us/windows/win32/api/mmeapi/ns-mmeapi-waveformatex
/// @see https://learn.microsoft.com/en-us/windows/win32/api/mmreg/ns-mmreg-waveformatextensible
/// @see https://learn.microsoft.com/en-us/windows/win32/api/audioclient/nn-audioclient-iaudiocaptureclient
/// 
/// @todo Watch the program with Process Monitor and make sure it's not
///       over-spinning any threads.  So far, it looks very good.
/// 
/// @author Mark Nelson <marknels@hawaii.edu>
/// @date   10_Oct_2022
///////////////////////////////////////////////////////////////////////////////

#include "framework.h"    // Standard system include files
#include <mmdeviceapi.h>  // For the audio API
#include <AudioClient.h>  // For the audio API
#include <Functiondiscoverykeys_devpkey.h>  // For some audio GUIDs
#include <strsafe.h>      // For sprintf_s
#include <avrt.h>         // For AvSetMmThreadCharacteristics
#include <inttypes.h>     // For printf to format fixed-integers

#include "audio.h"        // For yo bad self
#include "mvcModel.h"     // For the model
#include "goertzel.h"     // For goertzel_compute_dtmf_tones
#include "mvcView.h"      // For mvcViewRefreshWindow

#pragma comment(lib, "avrt")  // Link the MMCSS library


/// The share mode for the audio capture device.  It can be either
/// `SHARED` or `EXCLUSIVE`.  Because this was developed in a VM, I suspect
/// that VMWare won't allow exclusive access.  Therefore, all this program 
/// supports (for now) is `SHARED`
/// 
/// @todo Consider supporting `EXCLUSIVE` audio device access someday (Issue #14)
AUDCLNT_SHAREMODE gShareMode = AUDCLNT_SHAREMODE_SHARED;


IMMDevice*      gDevice         = NULL;  ///< COM object for a multimedia device
LPWSTR          glpwstrDeviceId = NULL;  ///< Device endpoint ID string `{0.0.1.00000000}.{722038ce-3a4e-4de0-8e7d-bd3fa6865a89}`
DWORD           glpdwState      =    0;  ///< The current device state `ACTIVE`, `DISABLED`, `NOT PRESENT` or `UNPLUGGED`
IPropertyStore* glpPropertyStore = NULL; ///< A multimedia device's property store
PROPVARIANT     gDeviceInterfaceFriendlyName;  ///< Container for the friendly name of the audio adapter for the device:  `High Definition Audio Device`
PROPVARIANT     gDeviceDescription;      ///< Container for the device's description:  `Microphone`
PROPVARIANT     gDeviceFriendlyName;     ///< Container for friendly name of the device:  `Microphone (High Definition Audio Device)`
WAVEFORMATEX*   gpMixFormat     = NULL;  ///< The internal audio format used by the device
WAVEFORMATEX*   gpAudioFormatUsed = NULL;///< The acutal format we will used by this app
IAudioClient*   glpAudioClient  = NULL;  ///< COM object for the audio client
REFERENCE_TIME  gDefaultDevicePeriod = -1; ///< Expressed in 100ns units (dev machine = 101,587 = 10.1587ms)
REFERENCE_TIME  gMinimumDevicePeriod = -1; ///< Expressed in 100ns units (dev machine = 29,025  = 2.9025ms
UINT32          guBufferSize    = 0;     ///< The maximum capacity of the endpoint buffer in frames = 182 frames
HANDLE          hCaptureThread  = NULL;  ///< The audio capture thread
IAudioCaptureClient* gCaptureClient = NULL; ///< The audio capture client

CHAR            sBuf[ 256 ];   ///< Debug buffer  @todo Put a guard around this
WCHAR           wsBuf[ 256 ];  ///< Debug buffer  @todo Put a guard around this


#ifdef MONITOR_PCM_AUDIO
   #define MONITOR_INTERVAL_SECONDS (4)   /**< The monitoring interval.  Set to 0 to disable monitoring */
   UINT64 gFramesToMonitor = 0;           ///< set #gbMonitor when the current frame is > #gStartOfMonitor + #gFramesToMonitor
   UINT64 gStartOfMonitor = UINT64_MAX;   ///< The frame position of the start time of the monitor
   BOOL   gbMonitor = false;              ///< Briefly set to 1 to output monitored data
   
   BYTE monitorCh1Max = 0;                ///< The lowest PCM value on Channel 1 during this monitoing period
   BYTE monitorCh1Min = 255;              ///< The highest PCM value on Channel 1 during this monitoring period
#endif


/// The audio formats DTMF_Decoder supports
enum audio_format_t {
   UNKNOWN_AUDIO_FORMAT=0,  ///< An unknown audio format   
   PCM_8,                   ///< 8-bit, linear PCM ranging from 0 to 255 where 0 is min, 127 is silence and 255 is max
   IEEE_FLOAT_32            ///< 32-bit float values from -1 to +1
};


/// The audio format DTMF_Decoder is currently using
audio_format_t audioFormat = UNKNOWN_AUDIO_FORMAT;


/// Process the audio frame, converting it into #PCM_8, adding the sample to
/// #pcmQueue and monitoring the values (if desired)
/// 
/// @param pData Pointer into the audio bufer
/// @param frame The frame number to process
/// @return `true` if successful.  `false` if there were problems.
BOOL processAudioFrame( BYTE* pData, UINT32 frame ) {
   _ASSERTE( pData != NULL );
   _ASSERTE( audioFormat != UNKNOWN_AUDIO_FORMAT );
   _ASSERTE( gpMixFormat != NULL );

   BYTE ch1Sample = PCM_8_BIT_SILENCE;

   switch ( audioFormat ) {
      case IEEE_FLOAT_32: {
            float* fSample = (float*) ( pData + ( frame * gpMixFormat->nBlockAlign ) );  // This is from +1 to -1

            INT8 signedSample = (INT8) ( *fSample * (float) PCM_8_BIT_SILENCE );  // This is +127 to -127
            if ( signedSample >= 0 ) {
               ch1Sample = signedSample + PCM_8_BIT_SILENCE;
            } else {
               ch1Sample = PCM_8_BIT_SILENCE + signedSample;  // -1, -2, -3, will turn into 127, 126, 125, ...
            }
            break;
         }
      case PCM_8:
         ch1Sample = *( pData + ( frame * gpMixFormat->nBlockAlign ) );
         break;
      default:
         _ASSERT_EXPR( FALSE, "Unknown audio format" );
   }

   pcmEnqueue( ch1Sample );

   #ifdef MONITOR_PCM_AUDIO
      // Optional code I use to characterize the samples by tracking the min and max
      // levels, peridoically printing them and then resetting them.  This way, I can
      // get a feel for what silence and various volumes look like in the data.  This
      // is an easy way to validate that the data I'm getting is real sound collected by
      // the microphone.
      if ( gFramesToMonitor > 0 ) {
         if ( ch1Sample > monitorCh1Max ) monitorCh1Max = ch1Sample;
         if ( ch1Sample < monitorCh1Min ) monitorCh1Min = ch1Sample;

         if ( gbMonitor ) {
            sprintf_s( sBuf, sizeof( sBuf ), "Channel 1:  Min: %" PRIu8 "   Max: %" PRIu8, monitorCh1Min, monitorCh1Max );
            OutputDebugStringA( sBuf );

            monitorCh1Max = 0;
            monitorCh1Min = 255;

            gbMonitor = false;
         }
      }
   #endif

   return TRUE;
}


/// Get audio frames from the device and process them
///
/// On virtualized systems, the hypervisor can play Merry Hell with this...
/// This is a realtime application.  Windows has several features to ensure
/// that realtime applications (like audio capture) have high proitory, but
/// the hypervisor doesn't really have visibility to that.
/// 
/// My observations are:  On bare-metal systems, the scheduling/performance of
/// this loop are OK.  On virtualized systems, you may see lots of 
/// DATA_DISCONTINUITY messages.  I assess this to be normal and outside of 
/// what I can program around.
/// 
/// When we get a DATA_DISCONTINUITY message, I'm choosing to drop the buffer.
/// I could just have easily processed it, but I'm thinking that I'll wait
/// for the scheduler to stabalize and only process 100% good buffers.  We also
/// throw out silent buffers.
/// 
/// If I did process discontinuous frames, I'd have the right frequency, but
/// I'd introduce phasing issues which could distort our results.
/// 
/// It's normal for the first buffer to have DATA_DISCONTINUITY set 
void audioCapture() {
   HRESULT hr;

   BYTE*  pData;
   UINT32 framesAvailable;
   DWORD  flags;
   UINT64 framePosition;

   _ASSERTE( gCaptureClient != NULL );
   _ASSERTE( audioFormat != UNKNOWN_AUDIO_FORMAT );

   // The following block of code is the core logic of DTMF_Decoder

   /// Use GetBuffer to get a bunch of frames of audio from the capture client
   hr = gCaptureClient->GetBuffer( &pData, &framesAvailable, &flags, &framePosition, NULL );
   if ( hr == S_OK ) {
      // OutputDebugStringA( __FUNCTION__ ":  I got data!" );

      if ( flags == 0 ) {
         // Normal processing
         for ( UINT32 i = 0 ; i < framesAvailable ; i++ ) {
            processAudioFrame( pData, i );  // Process each audio frame
         }
         
         /// After all of the frames have been queued, compute the DFT
         /// 
         /// Note:  This thread will wait, signal 8 DFT threads to run, then
         ///        will continue after the DFT threads are done
         goertzel_compute_dtmf_tones();
         
         /// If, after computing all 8 of the DFTs, if the detected state
         /// has changed, then refresh the main window
         if ( hasDtmfTonesChanged ) {
            mvcViewRefreshWindow();
         }
      } 

      /// Carefully analyze the flags returned by GetBuffer
      if ( flags & AUDCLNT_BUFFERFLAGS_SILENT ) {
         OutputDebugStringA( __FUNCTION__ ":  Buffer flag set:  SILENT" );
         // Nothing so see here.  Move along.
         flags &= !AUDCLNT_BUFFERFLAGS_SILENT;  // Clear AUDCLNT_BUFFERFLAGS_SILENT from flags
      } 
      if ( flags & AUDCLNT_BUFFERFLAGS_DATA_DISCONTINUITY ) {
         OutputDebugStringA( __FUNCTION__ ":  Buffer flag set:  DATA_DISCONTINUITY" );
         // Throw these packets out
         flags &= !AUDCLNT_BUFFERFLAGS_DATA_DISCONTINUITY;  // Clear AUDCLNT_BUFFERFLAGS_DATA_DISCONTINUITY from flags
      } 
      if ( flags & AUDCLNT_BUFFERFLAGS_TIMESTAMP_ERROR ) {
         OutputDebugStringA( __FUNCTION__ ":  Buffer flag set:  TIMESTAMP_ERROR" );
         // Throw this packet out as well
         flags &= !AUDCLNT_BUFFERFLAGS_TIMESTAMP_ERROR;  // Clear AUDCLNT_BUFFERFLAGS_TIMESTAMP_ERROR from flags
      }
      if ( flags != 0 ) {
         OutputDebugStringA( __FUNCTION__ ":  Some other bufer flags are set.  Investigate!" );
         // Throw this packet out as well
      }

      if ( framesAvailable > 0 ) {
         #ifdef MONITOR_PCM_AUDIO
            gbMonitor = false;
            if ( gFramesToMonitor > 0 ) {
               if ( gStartOfMonitor > framePosition ) {
                  gStartOfMonitor = framePosition;
               }
          
               if ( gStartOfMonitor + gFramesToMonitor < framePosition ) {
                  gbMonitor = true;
                  gStartOfMonitor = framePosition;
               }
          
            }
          
            if ( gbMonitor ) {  // Monitor data on this pass
               OutputDebugStringA( __FUNCTION__ ":  Monitoring loop" );
               sprintf_s( sBuf, sizeof( sBuf ), "Frames available=%" PRIu32 "    frame position=%" PRIu64, framesAvailable, framePosition );
               OutputDebugStringA( sBuf );
          
               memset( sBuf, 0, 1 );
          
               for ( int i = 0 ; i < NUMBER_OF_DTMF_TONES ; i++ ) {
                  sprintf_s( sBuf+strlen(sBuf), sizeof(sBuf), "  %4.0fHz=%4.2f", dtmfTones[i].frequency, dtmfTones[i].goertzelMagnitude);
               }
               OutputDebugStringA( sBuf );
            }
         #endif

         hr = gCaptureClient->ReleaseBuffer( framesAvailable );
         if ( hr != S_OK ) {
            OutputDebugStringA( __FUNCTION__ ":  ReleaseBuffer didn't return S_OK.  Investigate!!" );
            isRunning = false;
         }
      }

   } else if ( hr == AUDCLNT_S_BUFFER_EMPTY ) {
      OutputDebugStringA( __FUNCTION__ ":  GetBuffer returned an empty buffer.  Continue." );
   } else if ( hr == AUDCLNT_E_OUT_OF_ORDER ) {
      OutputDebugStringA( __FUNCTION__ ":  GetBuffer returned out of order data.  Continue." );
   } else {
      /// If the audio device changes (unplugged, for example) then GetBuffer
      /// will return something unexpected and we should see it here.  If this
      /// happens, set #isRunning to `false` and gracefully shutdown the app.

      OutputDebugStringA( __FUNCTION__ ":  GetBuffer did not return S_OK.  Investigate!!" );
      isRunning = false;
      SendMessage(ghMainWindow, WM_CLOSE, 0, 0);  // Shutdown the app
   }
}


/// This thread waits for the audio device to call us back when it has some
/// data to process.
/// 
/// @param Context Not used
/// @return Return `0` if successful.  `0xFFFF `if there was a problem.
DWORD WINAPI audioCaptureThread( LPVOID Context ) {
   OutputDebugStringA( __FUNCTION__ ":  Start capture thread" );

   HRESULT hr;
   HANDLE mmcssHandle = NULL;

   /// Initialize COM for the thread
   hr = CoInitializeEx( NULL, COINIT_MULTITHREADED );
   if ( hr != S_OK ) {
      OutputDebugStringA( __FUNCTION__ ":  Failed to initialize COM in thread" );
      ExitThread( 0xFFFF );
   }

   /// Set the multimedia class scheduler service, which will set the CPU
   /// priority for this thread
   mmcssHandle = AvSetMmThreadCharacteristics( L"Capture", &mmcssTaskIndex );
   if ( mmcssHandle == NULL ) {
      OutputDebugStringA( __FUNCTION__ ":  Failed to set MMCSS on the audio capture thread.  Continuing." );
   }
   OutputDebugStringA( __FUNCTION__ ":  Set MMCSS on thread." );

   #ifdef MONITOR_PCM_AUDIO
      gFramesToMonitor = MONITOR_INTERVAL_SECONDS * gpMixFormat->nSamplesPerSec;
   #endif

   /// Audio capture loop
   /// #isRunning gets set to false by WM_CLOSE
   
   isRunning = true;

   while ( isRunning ) {
      DWORD dwWaitResult;

      dwWaitResult = WaitForSingleObject( gAudioSamplesReadyEvent, INFINITE );
      if ( dwWaitResult == WAIT_OBJECT_0 ) {
         if ( isRunning ) {
            audioCapture();
         }
      } else if ( dwWaitResult == WAIT_FAILED ) {
         OutputDebugStringA( __FUNCTION__ ":  The wait was failed.  Exiting" );
         isRunning = false;
         break;  // While loop
      } else {
         OutputDebugStringA( __FUNCTION__ ":  The wait was ended for some other reason.  Exiting.  Investigate!" );
         isRunning = false;
         break;  // While loop
      }
   }

   // Done.  Time to cleanup the thread

   if ( mmcssHandle != NULL ) {
      if ( !AvRevertMmThreadCharacteristics( mmcssHandle ) ) {
         OutputDebugStringA( __FUNCTION__ ":  Failed to revert MMCSS on the audio capture thread.  Continuing." );
      }
      mmcssHandle = NULL;
   }

   CoUninitialize();

   OutputDebugStringA( __FUNCTION__ ":  End capture thread" );

   ExitThread( 0 );
}


/// Print the WAVEFORMATEX or WAVEFORMATEXTENSIBLE structure to OutputDebug
/// 
/// #### Sample Output
/**@verbatim
    audioPrintWaveFormat:  Using WAVE_FORMAT_EXTENSIBLE format
    Channels=2
    Samples per Second=44100
    Bytes per Second=352800
    Block (frame) alignment, in bytes=8
    Bits per sample=32
    Valid bits per sample=32
    Extended wave format is IEEE Float
@endverbatim */
BOOL audioPrintWaveFormat( WAVEFORMATEX* pFmt ) {
   if ( pFmt->wFormatTag == WAVE_FORMAT_EXTENSIBLE ) {
      OutputDebugStringA( "   " __FUNCTION__ ":  Using WAVE_FORMAT_EXTENSIBLE format");
      WAVEFORMATEXTENSIBLE* pFmtEx = (WAVEFORMATEXTENSIBLE*) pFmt;

      sprintf_s( sBuf, sizeof( sBuf ), "   Channels=%" PRIu16, pFmtEx->Format.nChannels );
      OutputDebugStringA( sBuf );

      sprintf_s( sBuf, sizeof( sBuf ), "   Samples per Second=%" PRIu32, pFmtEx->Format.nSamplesPerSec );
      OutputDebugStringA( sBuf );

      sprintf_s( sBuf, sizeof( sBuf ), "   Bytes per Second=%" PRIu32, pFmtEx->Format.nAvgBytesPerSec );
      OutputDebugStringA( sBuf );

      sprintf_s( sBuf, sizeof( sBuf ), "   Block (frame) alignment, in bytes=%" PRIu32, pFmtEx->Format.nBlockAlign );
      OutputDebugStringA( sBuf );

      sprintf_s( sBuf, sizeof( sBuf ), "   Bits per sample=%" PRIu32, pFmtEx->Format.wBitsPerSample );
      OutputDebugStringA( sBuf );

      sprintf_s( sBuf, sizeof( sBuf ), "   Valid bits per sample=%" PRIu16, pFmtEx->Samples.wValidBitsPerSample );
      OutputDebugStringA( sBuf );

      if ( pFmtEx->SubFormat == KSDATAFORMAT_SUBTYPE_PCM ) {
         OutputDebugStringA( "   Extended wave format is PCM" );
      } else if ( pFmtEx->SubFormat == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT ) {
         OutputDebugStringA( "   Extended wave format is IEEE Float" );
      } else {
         OutputDebugStringA( "   Extended wave format is not PCM" );
      }
   } else {
      OutputDebugStringA( "   " __FUNCTION__ ":  Using WAVE_FORMAT format");

      if ( pFmt->wFormatTag == WAVE_FORMAT_PCM ) {
         OutputDebugStringA( "   Wave format is PCM" );
      } else if ( pFmt->wFormatTag == WAVE_FORMAT_IEEE_FLOAT ) {
         OutputDebugStringA( "   Wave format is IEEE Float" );
      } else {
         OutputDebugStringA( "   Wave format is not PCM" );
      }

      sprintf_s( sBuf, sizeof( sBuf ), "   Channels=%" PRIu16, pFmt->nChannels );
      OutputDebugStringA( sBuf );

      sprintf_s( sBuf, sizeof( sBuf ), "   Samples per Second=%" PRIu32, pFmt->nSamplesPerSec );
      OutputDebugStringA( sBuf );

      sprintf_s( sBuf, sizeof( sBuf ), "   Bytes per Second=%" PRIu32, pFmt->nAvgBytesPerSec );
      OutputDebugStringA( sBuf );

      sprintf_s( sBuf, sizeof( sBuf ), "   Block (frame) alignment, in bytes=%" PRIu32, pFmt->nBlockAlign );
      OutputDebugStringA( sBuf );

      sprintf_s( sBuf, sizeof( sBuf ), "   Bits per sample=%" PRIu32, pFmt->wBitsPerSample );
      OutputDebugStringA( sBuf );
   }

   return TRUE;
}


/// Initialize the audio capture device and start the capture thread
/// 
/// @return `true` if successful.  `false` if there was a problem.
BOOL audioInit() {
   HRESULT hr;  // Result handle used by just about all Windows API calls

   if ( gShareMode == AUDCLNT_SHAREMODE_EXCLUSIVE ) {
      OutputDebugStringA( __FUNCTION__ ":  Exclusive mode not supported right now" );
      return FALSE;
   }

   /// Get IMMDeviceEnumerator from COM (CoCreateInstance)
   IMMDeviceEnumerator* deviceEnumerator = NULL;

   hr = CoCreateInstance( __uuidof( MMDeviceEnumerator ), NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS( &deviceEnumerator ) );
   CHECK_HR( "Failed to instantiate the multimedia device enumerator via COM" );

   /// Get the IMMDevice
   _ASSERTE( gDevice == NULL );

   hr = deviceEnumerator->GetDefaultAudioEndpoint( eCapture, eMultimedia, &gDevice );
   CHECK_HR( "Failed to get default audio device" );

   _ASSERTE( gDevice != NULL );

   /// Get the ID from IMMDevice
   hr = gDevice->GetId( &glpwstrDeviceId );
   if ( hr != S_OK || glpwstrDeviceId == NULL ) {
      OutputDebugStringA( __FUNCTION__ ":  Failed to get the device's ID string" );
      return FALSE;
   }

   swprintf_s( wsBuf, sizeof( wsBuf ), __FUNCTIONW__ L":  Device ID=%s", glpwstrDeviceId );
   OutputDebugStringW( wsBuf );

   /// Get the State from IMMDevice
   hr = gDevice->GetState( &glpdwState );
   if ( hr != S_OK || glpdwState == NULL ) {
      OutputDebugStringA( __FUNCTION__ ":  Failed to get the device's state" );
      return FALSE;
   }

   if ( glpdwState != DEVICE_STATE_ACTIVE ) {
      OutputDebugStringA( __FUNCTION__ ":  The audio device state is not active" );
      return FALSE;
   }

   /// Get the Property Store from IMMDevice
   hr = gDevice->OpenPropertyStore( STGM_READ, &glpPropertyStore );
   if ( hr != S_OK || glpPropertyStore == NULL ) {
      OutputDebugStringA( __FUNCTION__ ":  Failed to open device property store" );
      return FALSE;
   }

   /// Get the device's properties from the property store
   PropVariantInit( &gDeviceInterfaceFriendlyName );     /// Get the friendly name of the audio adapter for the device
   PropVariantInit( &gDeviceDescription );               /// Get the device's description
   PropVariantInit( &gDeviceFriendlyName );              /// Get the friendly name of the device

   hr = glpPropertyStore->GetValue( PKEY_DeviceInterface_FriendlyName, &gDeviceInterfaceFriendlyName );
   if ( hr != S_OK ) {
      OutputDebugStringA( __FUNCTION__ ":  Failed to retrieve the friendly name of the audio adapter for the device.  Continuing." );
      gDeviceInterfaceFriendlyName.pcVal = NULL;
   } else {
      swprintf_s( wsBuf, sizeof( wsBuf ), __FUNCTIONW__ L":  Device audio adapter friendly name=%s", gDeviceInterfaceFriendlyName.pwszVal );
      OutputDebugStringW( wsBuf );
   }

   hr = glpPropertyStore->GetValue( PKEY_Device_DeviceDesc, &gDeviceDescription );
   if ( hr != S_OK ) {
      OutputDebugStringA( __FUNCTION__ ":  Failed to retrieve the device's description.  Continuing." );
      gDeviceDescription.pcVal = NULL;
   } else {
      swprintf_s( wsBuf, sizeof( wsBuf ), __FUNCTIONW__ L":  Device description=%s", gDeviceDescription.pwszVal );
      OutputDebugStringW( wsBuf );
   }

   hr = glpPropertyStore->GetValue( PKEY_Device_FriendlyName, &gDeviceFriendlyName );
   if ( hr != S_OK ) {
      OutputDebugStringA( __FUNCTION__ ":  Failed to retrieve the friendly name of the device.  Continuing." );
      gDeviceFriendlyName.pcVal = NULL;
   } else {
      swprintf_s( wsBuf, sizeof( wsBuf ), __FUNCTIONW__ L":  Device friendly name=%s", gDeviceFriendlyName.pwszVal );
      OutputDebugStringW( wsBuf );
   }

   /// Use Activate on IMMDevice to create an IAudioClient
   hr = gDevice->Activate( __uuidof( IAudioClient ), CLSCTX_ALL, NULL, (void**) &glpAudioClient );
   if ( hr != S_OK || glpAudioClient == NULL ) {
      OutputDebugStringA( __FUNCTION__ ":  Failed to create an audio client" );
      return FALSE;
   }

   /// Get the default audio format that the audio driver wants to use
   hr = glpAudioClient->GetMixFormat( &gpMixFormat );
   if ( hr != S_OK || gpMixFormat == NULL ) {
      OutputDebugStringA( __FUNCTION__ ":  Failed to retrieve mix format" );
      return FALSE;
   }

   OutputDebugStringA( __FUNCTION__ ":  The mix format follows" );
   audioPrintWaveFormat( gpMixFormat );

   hr = glpAudioClient->IsFormatSupported( gShareMode, gpMixFormat, &gpAudioFormatUsed );
   if ( hr == S_OK ) {
      OutputDebugStringA( __FUNCTION__ ":  The requested format is supported");
   } else if ( hr == AUDCLNT_E_UNSUPPORTED_FORMAT ) {
      OutputDebugStringA( __FUNCTION__ ":  The requested format is is not supported" );
      return FALSE;
   } else if( hr == S_FALSE && gpAudioFormatUsed != NULL) {
      OutputDebugStringA( __FUNCTION__ ":  The requested format is not available, but this format is..." );
      audioPrintWaveFormat( gpAudioFormatUsed );
      return FALSE;
   } else {
      OutputDebugStringA( __FUNCTION__ ":  Failed to validate the requested format" );
      return FALSE;
   }

   /// Determine the audio format
   audioFormat = UNKNOWN_AUDIO_FORMAT;

   if ( gpMixFormat->wFormatTag == WAVE_FORMAT_PCM && gpMixFormat->wBitsPerSample == 8 ) {
      audioFormat = PCM_8;
   } else if ( gpMixFormat->wFormatTag == WAVE_FORMAT_IEEE_FLOAT && gpMixFormat->wBitsPerSample == 32 ) {
      audioFormat = IEEE_FLOAT_32;
   } else if ( gpMixFormat->wFormatTag == WAVE_FORMAT_EXTENSIBLE ) {
      WAVEFORMATEXTENSIBLE* pFmtEx = (WAVEFORMATEXTENSIBLE*) gpMixFormat;

      if ( pFmtEx->SubFormat == KSDATAFORMAT_SUBTYPE_PCM && pFmtEx->Samples.wValidBitsPerSample == 8 ) {
         audioFormat = PCM_8;
      } else if ( pFmtEx->SubFormat == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT && pFmtEx->Samples.wValidBitsPerSample == 32 ) {
         audioFormat = IEEE_FLOAT_32;
      }
   }

   if ( audioFormat == UNKNOWN_AUDIO_FORMAT ) {
      OutputDebugStringA( __FUNCTION__ ":  Failed to match with the audio format" );
      return FALSE;
   }

   _ASSERTE( audioFormat != UNKNOWN_AUDIO_FORMAT );

   /// Initialize shared mode audio client
   //  Shared mode streams using event-driven buffering must set both periodicity and bufferDuration to 0.
   hr = glpAudioClient->Initialize( gShareMode, AUDCLNT_STREAMFLAGS_EVENTCALLBACK
                                              | AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM, 0, 0, gpMixFormat, NULL );
   if ( hr != S_OK ) {
      /// @todo Look at more error codes and print out higher-fidelity error messages
      OutputDebugStringA( __FUNCTION__ ":  Failed to initialize the audio client" );
      return FALSE;
   }

   hr = glpAudioClient->GetBufferSize( &guBufferSize );
   CHECK_HR( "Failed to get buffer size" );
   sprintf_s( sBuf, sizeof( sBuf ), "%s:  The maximum capacity of the buffer is %" PRIu32" frames or %i ms", __FUNCTION__, guBufferSize, (int) (1.0 / gpMixFormat->nSamplesPerSec * 1000 * guBufferSize ) );
   OutputDebugStringA( sBuf );
   /// Right now, the buffer is ~22ms or about the perfect size to capture
   /// VoIP voice, which is 20ms.

   /// Get the device period
   hr = glpAudioClient->GetDevicePeriod( &gDefaultDevicePeriod, &gMinimumDevicePeriod );
   CHECK_HR( "Failed to get audio client device periods" );

   sprintf_s( sBuf, sizeof( sBuf ), "%s:  Default device period=%lli ms", __FUNCTION__, gDefaultDevicePeriod/10000 );
   OutputDebugStringA( sBuf );

   sprintf_s( sBuf, sizeof( sBuf ), "%s:  Minimum device period=%lli ms", __FUNCTION__, gMinimumDevicePeriod/10000 );
   OutputDebugStringA( sBuf );


   /// Initialize the DTMF buffer
   if ( pcmSetQueueSize( gpMixFormat->nSamplesPerSec / 1000 * SIZE_OF_QUEUE_IN_MS ) != TRUE ) {
      OutputDebugStringA( __FUNCTION__ ":  Failed to allocate PCM queue" );
      return FALSE;
   }

   sprintf_s( sBuf, sizeof( sBuf ), "%s:  Queue size=%zu bytes or %d ms", __FUNCTION__, queueSize, SIZE_OF_QUEUE_IN_MS );
   OutputDebugStringA( sBuf );

   /// Initialize the Goertzel module
   isRunning = true;
   if ( goertzel_init( gpMixFormat->nSamplesPerSec ) != TRUE ) {
      OutputDebugStringA( __FUNCTION__ ":  Failed to initialioze Goertzel Function" );
      return FALSE;
   }


   /// Create the callback events
   gAudioSamplesReadyEvent = CreateEventEx( NULL, NULL, 0, EVENT_MODIFY_STATE | SYNCHRONIZE );
   if ( gAudioSamplesReadyEvent == NULL ) {
      OutputDebugStringA( __FUNCTION__ ":  Failed to create audio samples ready event" );
      return FALSE;
   }

   hr = glpAudioClient->SetEventHandle( gAudioSamplesReadyEvent );
   CHECK_HR( "Failed to set audio capture ready event" );

   /// Get the Capture Client
   hr = glpAudioClient->GetService( IID_PPV_ARGS( &gCaptureClient ) );
   CHECK_HR( "Failed to get capture client" )

   /// Start the thread
   hCaptureThread = CreateThread( NULL, 0, audioCaptureThread, NULL, 0, NULL );
   if ( hCaptureThread == NULL ) {
      OutputDebugStringA( __FUNCTION__ ":  Failed to create the capture thread" );
      return FALSE;
   }

   /// Start the audio processer
   hr = glpAudioClient->Start();
   CHECK_HR( "Failed to start capturing the audio stream" );

   OutputDebugStringA( __FUNCTION__ ":  The audio capture interface has been initialized" );

   /// The thread of execution goes back to #wWinMain, which starts the main 
   /// message loop
   return TRUE;
}


/// Stop the audio device
/// @return `true` if successful.  `false` if there was a problem.
BOOL audioStopDevice() {
   HRESULT hr;

   if ( glpAudioClient != NULL ) {
      hr = glpAudioClient->Stop();
      CHECK_HR( "Stopping the audio stream returned an odd value.  Investigate!!" );
   }

   return TRUE;
}


/// Cleanup all things audio.  Basically unwind everything that was done in
/// #audioInit
/// 
/// @return `true` if successful.  `false` if there was a problem.
BOOL audioCleanup() {
   BOOL    br;  // BOOL result
   HRESULT hr;  // HRESULT result

   if ( hCaptureThread != NULL ) {
      br = CloseHandle( hCaptureThread );
      CHECK_BR( "Failed to close hCaptureThread" );
      hCaptureThread = NULL;
   }

   SAFE_RELEASE( gCaptureClient );

   if ( gAudioSamplesReadyEvent != NULL ) {
      br = CloseHandle( gAudioSamplesReadyEvent );
      CHECK_BR( "Failed to close gAudioSamplesReadyEvent" );
      gAudioSamplesReadyEvent = NULL;
   }

   if ( glpAudioClient != NULL ) {
      hr = glpAudioClient->Reset();
      CHECK_HR( "Failed to release the audio client")
      glpAudioClient = NULL;
   }

   pcmReleaseQueue();

   SAFE_RELEASE( glpAudioClient );

   hr = PropVariantClear( &gDeviceFriendlyName );
   CHECK_HR( "Failed to release gDeviceFriendlyName" );
   PropVariantClear( &gDeviceDescription );
   CHECK_HR( "Failed to release gDeviceDescription" );
   PropVariantClear( &gDeviceInterfaceFriendlyName );
   CHECK_HR( "Failed to release gDeviceInterfaceFriendlyName" );

   SAFE_RELEASE( glpPropertyStore );

   if ( gpMixFormat != NULL ) {
      CoTaskMemFree( gpMixFormat );
      gpMixFormat = NULL;
   }

   if ( gpAudioFormatUsed != NULL ) {
      CoTaskMemFree( gpAudioFormatUsed );
      gpAudioFormatUsed = NULL;
   }

   if ( glpwstrDeviceId != NULL ) {
      CoTaskMemFree( glpwstrDeviceId );
      glpwstrDeviceId = NULL;
   }

   SAFE_RELEASE( gDevice );

   return TRUE;
}
