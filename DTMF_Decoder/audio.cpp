///////////////////////////////////////////////////////////////////////////////
//          University of Hawaii, College of Engineering
//          DTMF_Decoder - EE 469 - Fall 2022
//
//  A Windows Desktop C program that decodes DTMF tones
//
/// Windows Audio Driver code
///
/// @see https://learn.microsoft.com/en-us/windows/win32/api/_coreaudio/
///
/// @see https://learn.microsoft.com/en-us/windows/win32/api/mmdeviceapi/nn-mmdeviceapi-immdevice
/// @see https://learn.microsoft.com/en-us/windows/win32/api/audioclient/nn-audioclient-iaudioclient
/// @see https://learn.microsoft.com/en-us/windows/win32/api/audioclient/nf-audioclient-iaudioclient-initialize
/// @see https://learn.microsoft.com/en-us/windows/win32/api/audioclient/nf-audioclient-iaudioclient-getbuffersize
/// @see https://learn.microsoft.com/en-us/windows/win32/api/propsys/nn-propsys-ipropertystore
/// @see https://learn.microsoft.com/en-us/windows/win32/api/mmeapi/ns-mmeapi-waveformatex
/// @see https://learn.microsoft.com/en-us/windows/win32/api/mmreg/ns-mmreg-waveformatextensible
/// @see https://learn.microsoft.com/en-us/windows/win32/api/audioclient/nn-audioclient-iaudiocaptureclient
///
/// @see https://learn.microsoft.com/en-us/windows/win32/api/audioclient/nf-audioclient-iaudiocaptureclient-getbuffer
/// @see https://learn.microsoft.com/en-us/windows/win32/api/audioclient/nf-audioclient-iaudiocaptureclient-releasebuffer
/// @see https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-sendmessage
/// @see https://learn.microsoft.com/en-us/windows/win32/api/combaseapi/nf-combaseapi-coinitializeex
/// @see https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-exitthread
/// @see https://learn.microsoft.com/en-us/windows/win32/api/combaseapi/nf-combaseapi-couninitialize
/// @see https://learn.microsoft.com/en-us/windows/win32/api/combaseapi/nf-combaseapi-cocreateinstance
/// @see https://learn.microsoft.com/en-us/windows/win32/api/mmdeviceapi/nf-mmdeviceapi-immdeviceenumerator-getdefaultaudioendpoint
/// @see https://learn.microsoft.com/en-us/windows/win32/api/mmdeviceapi/nf-mmdeviceapi-immdevice-getid
/// @see https://learn.microsoft.com/en-us/windows/win32/api/mmdeviceapi/nf-mmdeviceapi-immdevice-getstate
/// @see https://learn.microsoft.com/en-us/windows/win32/api/mmdeviceapi/nf-mmdeviceapi-immdevice-openpropertystore
/// @see https://learn.microsoft.com/en-us/windows/win32/api/propidl/nf-propidl-propvariantinit
/// @see https://learn.microsoft.com/en-us/previous-versions/windows/desktop/legacy/bb761473(v=vs.85)
/// @see https://learn.microsoft.com/en-us/windows/win32/api/mmdeviceapi/nf-mmdeviceapi-immdevice-activate
/// @see https://learn.microsoft.com/en-us/windows/win32/api/audioclient/nf-audioclient-iaudioclient-getmixformat
/// @see https://learn.microsoft.com/en-us/windows/win32/api/audioclient/nf-audioclient-iaudioclient-isformatsupported
/// @see https://learn.microsoft.com/en-us/windows/win32/api/audioclient/nf-audioclient-iaudioclient-initialize
/// @see https://learn.microsoft.com/en-us/windows/win32/api/audioclient/nf-audioclient-iaudioclient-getbuffersize
/// @see https://learn.microsoft.com/en-us/windows/win32/api/audioclient/nf-audioclient-iaudioclient-getdeviceperiod
/// @see https://learn.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-createeventexa
/// @see https://learn.microsoft.com/en-us/windows/win32/api/audioclient/nf-audioclient-iaudioclient-seteventhandle
/// @see https://learn.microsoft.com/en-us/windows/win32/api/audioclient/nf-audioclient-iaudioclient-getservice
/// @see https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-createthread
/// @see https://learn.microsoft.com/en-us/windows/win32/api/audioclient/nf-audioclient-iaudioclient-start
/// @see https://learn.microsoft.com/en-us/windows/win32/api/audioclient/nf-audioclient-iaudioclient-stop
/// @see https://learn.microsoft.com/en-us/windows/win32/api/unknwn/nf-unknwn-iunknown-release
/// @see https://learn.microsoft.com/en-us/windows/win32/api/audioclient/nf-audioclient-iaudioclient-reset
/// @see https://learn.microsoft.com/en-us/windows/win32/api/combaseapi/nf-combaseapi-propvariantclear
/// @see https://learn.microsoft.com/en-us/windows/win32/api/combaseapi/nf-combaseapi-cotaskmemfree
/// @see https://learn.microsoft.com/en-us/windows/win32/api/avrt/nf-avrt-avsetmmthreadcharacteristicsa
/// @see https://learn.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-waitforsingleobject
/// @see https://learn.microsoft.com/en-us/windows/win32/api/avrt/nf-avrt-avrevertmmthreadcharacteristics
/// @see https://learn.microsoft.com/en-us/windows/win32/api/handleapi/nf-handleapi-closehandle
///
/// @todo Watch the program with Process Monitor and make sure it's not
///       over-spinning any threads.  So far, it looks very good.
///
/// @file    audio.cpp
/// @version 1.0
///
/// @author  Mark Nelson <marknels@hawaii.edu>
/// @date    10_Oct_2022
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
static AUDCLNT_SHAREMODE sShareMode = AUDCLNT_SHAREMODE_SHARED;

static IMMDevice*      spDevice             = NULL; ///< COM object for a multimedia device
static LPWSTR          spwstrDeviceId       = NULL; ///< Device endpoint ID string `{0.0.1.00000000}.{722038ce-3a4e-4de0-8e7d-bd3fa6865a89}`
static DWORD           sdwState             =    0; ///< The current device state `ACTIVE`, `DISABLED`, `NOT PRESENT` or `UNPLUGGED`
static IPropertyStore* spPropertyStore      = NULL; ///< A multimedia device's property store
static PROPVARIANT     sDeviceInterfaceFriendlyName; ///< Container for the friendly name of the audio adapter for the device:  `High Definition Audio Device`
static PROPVARIANT     sDeviceDescription;          ///< Container for the device's description:  `Microphone`
static PROPVARIANT     sDeviceFriendlyName;         ///< Container for friendly name of the device:  `Microphone (High Definition Audio Device)`
static WAVEFORMATEX*   spMixFormat          = NULL; ///< The internal audio format used by the device
static WAVEFORMATEX*   spAudioFormatUsed    = NULL; ///< The acutal format we will used by this app
static IAudioClient*   spAudioClient        = NULL; ///< COM object for the audio client
static REFERENCE_TIME  sDefaultDevicePeriod =   -1; ///< Expressed in 100ns units (dev machine = 101,587 = 10.1587ms)
static REFERENCE_TIME  sMinimumDevicePeriod =   -1; ///< Expressed in 100ns units (dev machine = 29,025  = 2.9025ms
static UINT32          suBufferSize         =    0; ///< The maximum capacity of the endpoint buffer in frames = 182 frames
static HANDLE          shCaptureThread      = NULL; ///< The audio capture thread
static IAudioCaptureClient* spCaptureClient = NULL; ///< The audio capture client


#ifdef MONITOR_PCM_AUDIO
   #define MONITOR_INTERVAL_SECONDS (4)   /**< The monitoring interval.  Set to 0 to disable monitoring */
   static UINT64 suFramesToMonitor = 0;          ///< set #sbMonitor when the current frameIndex is > #suStartOfMonitor + #suFramesToMonitor
   static UINT64 suStartOfMonitor  = UINT64_MAX; ///< The frameIndex position of the start time of the monitor
   static BOOL   sbMonitor         = false;      ///< Briefly set to 1 to output monitored data

   static BYTE   suMonitorCh1Max   = 0;          ///< The lowest PCM value on Channel 1 during this monitoing period
   static BYTE   suMonitorCh1Min   = 255;        ///< The highest PCM value on Channel 1 during this monitoring period
#endif


/// The audio formats DTMF_Decoder supports
enum audio_format_t {
   UNKNOWN_AUDIO_FORMAT=0,  ///< An unknown audio format
   PCM_8,                   ///< 8-bit, linear PCM ranging from 0 to 255 where 0 is min, 127 is silence and 255 is max
   IEEE_FLOAT_32            ///< 32-bit float values from -1 to +1
};


/// The audio format DTMF_Decoder is currently using
static audio_format_t sAudioFormat = UNKNOWN_AUDIO_FORMAT;


/// Process the audio frameIndex, converting it into #PCM_8, adding the sample to
/// #gPcmQueue and monitoring the values (if desired)
///
/// @param pData      Pointer to the head of the audio bufer
/// @param frameIndex The frameIndex number to process
/// @return `true` if successful.  `false` if there were problems.
BOOL processAudioFrame(
   _In_     BYTE*    pData,
   _In_     UINT32   frameIndex ) {

   _ASSERTE( pData != NULL );
   _ASSERTE( sAudioFormat != UNKNOWN_AUDIO_FORMAT );
   _ASSERTE( spMixFormat != NULL );

   BYTE ch1Sample = PCM_8_BIT_SILENCE;

   switch ( sAudioFormat ) {
      case IEEE_FLOAT_32: {
            float* fSample = (float*) ( pData + ( frameIndex * spMixFormat->nBlockAlign ) );  // This is from +1 to -1

            INT8 signedSample = (INT8) ( *fSample * (float) PCM_8_BIT_SILENCE );  // This is +127 to -127
            if ( signedSample >= 0 ) {
               ch1Sample = signedSample + PCM_8_BIT_SILENCE;
            } else {
               ch1Sample = PCM_8_BIT_SILENCE + signedSample;  // -1, -2, -3, will turn into 127, 126, 125, ...
            }
            break;
         }
      case PCM_8:
         ch1Sample = *( pData + ( frameIndex * spMixFormat->nBlockAlign ) );
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
      if ( suFramesToMonitor > 0 ) {
         if ( ch1Sample > suMonitorCh1Max ) suMonitorCh1Max = ch1Sample;
         if ( ch1Sample < suMonitorCh1Min ) suMonitorCh1Min = ch1Sample;

         if ( sbMonitor ) {
            LOG_TRACE( "Channel 1:  Min: %" PRIu8 "   Max: %" PRIu8, suMonitorCh1Min, suMonitorCh1Max );

            suMonitorCh1Max = 0;
            suMonitorCh1Min = 255;

            sbMonitor = false;
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
   HRESULT hr;  // HRESULT result
   BOOL    br;  // BOOL result


   BYTE*   pData;
   UINT32  framesAvailable;
   DWORD   flags;
   UINT64  framePosition;

   _ASSERTE( spCaptureClient != NULL );
   _ASSERTE( sAudioFormat != UNKNOWN_AUDIO_FORMAT );

   // The following block of code is the core logic of DTMF_Decoder

   /// Use GetBuffer to get a bunch of frames of audio from the capture client
   hr = spCaptureClient->GetBuffer( &pData, &framesAvailable, &flags, &framePosition, NULL );
   if ( hr == S_OK ) {
      // LOG_TRACE( "I got data!" );
      _ASSERTE( pData != NULL );

      if ( flags == 0 ) {
         // Normal processing
         for ( UINT32 i = 0 ; i < framesAvailable ; i++ ) {
            processAudioFrame( pData, i );  // Process each audio frameIndex
         }

         /// Make sure #gPcmQueue is healthy
         _ASSERTE( _CrtCheckMemory() );

         /// After all of the frames have been queued, compute the DFT
         ///
         /// Note:  This thread will wait, signal 8 DFT threads to run, then
         ///        will continue after the DFT threads are done
         br = goertzel_compute_dtmf_tones();
         if ( !br ) {
            LOG_FATAL( "Failed to compute DTMF tones.  Exiting.  Investigate!" );
            gracefulShutdown();
         }

         /// If, after computing all 8 of the DFTs, if the detected state
         /// has changed, then refresh the main window
         if ( gbHasDtmfTonesChanged ) {
            br = mvcViewRefreshWindow();
            if ( !br ) {
               LOG_FATAL( "Failed to refresh the main window.  Exiting.  Investigate!" );
               gracefulShutdown();
            }
         }
      }

      /// Carefully analyze the flags returned by GetBuffer
      if ( flags & AUDCLNT_BUFFERFLAGS_SILENT ) {
         LOG_INFO( "Buffer flag set: SILENT" );
         // Nothing so see here.  Move along.
         flags &= !AUDCLNT_BUFFERFLAGS_SILENT;  // Clear AUDCLNT_BUFFERFLAGS_SILENT from flags
      }
      if ( flags & AUDCLNT_BUFFERFLAGS_DATA_DISCONTINUITY ) {
         LOG_INFO( "Buffer flag set: DATA_DISCONTINUITY" );
         // Throw these packets out
         flags &= !AUDCLNT_BUFFERFLAGS_DATA_DISCONTINUITY;  // Clear AUDCLNT_BUFFERFLAGS_DATA_DISCONTINUITY from flags
      }
      if ( flags & AUDCLNT_BUFFERFLAGS_TIMESTAMP_ERROR ) {
         LOG_INFO( "Buffer flag set: TIMESTAMP_ERROR" );
         // Throw these packets out as well
         flags &= !AUDCLNT_BUFFERFLAGS_TIMESTAMP_ERROR;  // Clear AUDCLNT_BUFFERFLAGS_TIMESTAMP_ERROR from flags
      }
      if ( flags != 0 ) {
         LOG_WARN( ":  Some other bufer flags are set.  Investigate!" );
         // Throw these packets
      }

      if ( framesAvailable > 0 ) {
         #ifdef MONITOR_PCM_AUDIO
            sbMonitor = false;
            if ( suFramesToMonitor > 0 ) {
               if ( suStartOfMonitor > framePosition ) {
                  suStartOfMonitor = framePosition;
               }

               if ( suStartOfMonitor + suFramesToMonitor < framePosition ) {
                  sbMonitor = true;
                  suStartOfMonitor = framePosition;
               }

            }

            /// If #MONITOR_INTERVAL_SECONDS is on, the monitoring output will produce:
/**@verbatim
audioCapture: Frames available=448  frame position=2118098   697Hz=0.02   770Hz=0.01   852Hz=0.02   941Hz=0.01  1209Hz=0.03  1336Hz=0.02  1477Hz=0.00  1633Hz=0.01
processAudioFrame: Channel 1:  Min: 125   Max: 129
@endverbatim */

            if ( sbMonitor ) {  // Monitor data on this pass
               LOG_TRACE( "Frames available=%" PRIu32 "  frame position=%" PRIu64
                          "  %4.0fHz=%4.2f  %4.0fHz=%4.2f  %4.0fHz=%4.2f  %4.0fHz=%4.2f"
                          "  %4.0fHz=%4.2f  %4.0fHz=%4.2f  %4.0fHz=%4.2f  %4.0fHz=%4.2f",
                  framesAvailable, framePosition,
                  gDtmfTones[ 0 ].frequency, gDtmfTones[ 0 ].goertzelMagnitude,
                  gDtmfTones[ 1 ].frequency, gDtmfTones[ 1 ].goertzelMagnitude,
                  gDtmfTones[ 2 ].frequency, gDtmfTones[ 2 ].goertzelMagnitude,
                  gDtmfTones[ 3 ].frequency, gDtmfTones[ 3 ].goertzelMagnitude,
                  gDtmfTones[ 4 ].frequency, gDtmfTones[ 4 ].goertzelMagnitude,
                  gDtmfTones[ 5 ].frequency, gDtmfTones[ 5 ].goertzelMagnitude,
                  gDtmfTones[ 6 ].frequency, gDtmfTones[ 6 ].goertzelMagnitude,
                  gDtmfTones[ 7 ].frequency, gDtmfTones[ 7 ].goertzelMagnitude
                  );
            }
         #endif

         hr = spCaptureClient->ReleaseBuffer( framesAvailable );
         if ( hr != S_OK ) {
            LOG_FATAL( "ReleaseBuffer didn't return S_OK.  Exiting.  Investigate!" );
            gracefulShutdown();
         }
      }

   } else if ( hr == AUDCLNT_S_BUFFER_EMPTY ) {
      LOG_INFO( "GetBuffer returned an empty buffer.  Continue." );
   } else if ( hr == AUDCLNT_E_OUT_OF_ORDER ) {
      LOG_INFO( "GetBuffer returned out of order data.  Continue." );
   } else {
      /// If the audio device changes (unplugged, for example) then GetBuffer
      /// will return something unexpected and we should see it here.  If this
      /// happens, gracefully shutdown the app.

      LOG_FATAL( "GetBuffer did not return S_OK.  Exiting.  Investigate!" );
      gracefulShutdown();
   }
}


/// This thread waits for the audio device to call us back when it has some
/// data to process.
///
/// @see https://learn.microsoft.com/en-us/previous-versions/windows/desktop/legacy/ms686736(v=vs.85)
///
/// @param Context Not used
/// @return Return `0` if successful.  `0xFFFF `if there was a problem.
DWORD WINAPI audioCaptureThread( LPVOID Context ) {
   LOG_TRACE( "Start capture thread" );

   HRESULT hr;                  // HRESULT result
   HANDLE  mmcssHandle = NULL;

   /// Initialize COM for the thread
   hr = CoInitializeEx( NULL, COINIT_MULTITHREADED );
   if ( hr != S_OK ) {
      LOG_FATAL( "Failed to initialize COM in thread." );
      ExitThread( 0xFFFF );
   }

   /// Set the multimedia class scheduler service, which will set the CPU
   /// priority for this thread
   mmcssHandle = AvSetMmThreadCharacteristics( L"Capture", &gdwMmcssTaskIndex );
   if ( mmcssHandle == NULL ) {
      LOG_WARN( "Failed to set MMCSS on the audio capture thread.  Continuing." );
   }
   LOG_TRACE( "Set MMCSS on the audio capture thread." );

   #ifdef MONITOR_PCM_AUDIO
      suFramesToMonitor = MONITOR_INTERVAL_SECONDS * spMixFormat->nSamplesPerSec;
   #endif

   /// Audio capture loop
   /// #gbIsRunning gets set to false by WM_CLOSE

   gbIsRunning = true;

   while ( gbIsRunning ) {
      DWORD dwWaitResult;

      dwWaitResult = WaitForSingleObject( ghAudioSamplesReadyEvent, INFINITE );
      if ( dwWaitResult == WAIT_OBJECT_0 ) {
         if ( gbIsRunning ) {
            audioCapture();
         }
      } else if ( dwWaitResult == WAIT_FAILED ) {
         LOG_FATAL( "WaitForSingleObject in audio capture thread failed.  Exiting.  Investigate!" );
         gracefulShutdown();
         break;  // While loop
      } else {
         LOG_FATAL( "WaitForSingleObject in audio capture thread failed.  Exiting.  Investigate!" );
         gracefulShutdown();
         break;  // While loop
      }
   }

   // Done.  Time to cleanup the thread

   if ( mmcssHandle != NULL ) {
      if ( !AvRevertMmThreadCharacteristics( mmcssHandle ) ) {
         LOG_WARN( "Failed to revert MMCSS on the audio capture thread.  Continuing." );
      }
      mmcssHandle = NULL;
   }

   CoUninitialize();

   LOG_TRACE( "End capture thread" );

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
    Block (frameIndex) alignment, in bytes=8
    Bits per sample=32
    Valid bits per sample=32
    Extended wave format is IEEE Float
@endverbatim */
BOOL audioPrintWaveFormat( _In_ WAVEFORMATEX* pFmt ) {
   _ASSERTE( pFmt != NULL );

   if ( pFmt->wFormatTag == WAVE_FORMAT_EXTENSIBLE ) {
      LOG_DEBUG( "Using WAVE_FORMAT_EXTENSIBLE format");
      WAVEFORMATEXTENSIBLE* pFmtEx = (WAVEFORMATEXTENSIBLE*) pFmt;

      LOG_DEBUG( "Channels=%" PRIu16,              pFmtEx->Format.nChannels );
      LOG_DEBUG( "Samples per Second=%" PRIu32,    pFmtEx->Format.nSamplesPerSec );
      LOG_DEBUG( "Bytes per Second=%" PRIu32,      pFmtEx->Format.nAvgBytesPerSec );
      LOG_DEBUG( "Block (frame) alignment, in bytes=%" PRIu32, pFmtEx->Format.nBlockAlign );
      LOG_DEBUG( "Bits per sample=%" PRIu32,       pFmtEx->Format.wBitsPerSample );
      LOG_DEBUG( "Valid bits per sample=%" PRIu16, pFmtEx->Samples.wValidBitsPerSample );

      if ( pFmtEx->SubFormat == KSDATAFORMAT_SUBTYPE_PCM ) {
         LOG_DEBUG( "Extended wave format is PCM" );
      } else if ( pFmtEx->SubFormat == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT ) {
         LOG_DEBUG( "Extended wave format is IEEE Float" );
      } else {
         LOG_DEBUG( "Extended wave format is not PCM" );
      }
   } else {
      LOG_DEBUG( "Using WAVE_FORMAT format");

      if ( pFmt->wFormatTag == WAVE_FORMAT_PCM ) {
         LOG_DEBUG( "Wave format is PCM" );
      } else if ( pFmt->wFormatTag == WAVE_FORMAT_IEEE_FLOAT ) {
         LOG_DEBUG( "Wave format is IEEE Float" );
      } else {
         LOG_DEBUG( "Wave format is not PCM" );
      }

      LOG_DEBUG( "Channels=%" PRIu16,           pFmt->nChannels );
      LOG_DEBUG( "Samples per Second=%" PRIu32, pFmt->nSamplesPerSec );
      LOG_DEBUG( "Bytes per Second=%" PRIu32,   pFmt->nAvgBytesPerSec );
      LOG_DEBUG( "Block (frame) alignment, in bytes=%" PRIu32, pFmt->nBlockAlign );
      LOG_DEBUG( "Bits per sample=%" PRIu32,    pFmt->wBitsPerSample );
   }

   return TRUE;
}


/// Initialize the audio capture device and start the capture thread
///
/// @return `true` if successful.  `false` if there was a problem.
BOOL audioInit() {
   HRESULT hr;  // HRESULT result
   BOOL    br;  // BOOL result

   if ( sShareMode == AUDCLNT_SHAREMODE_EXCLUSIVE ) {
      LOG_FATAL( "Exclusive mode not supported right now" );
      return FALSE;
   }

   _ASSERTE( sShareMode == AUDCLNT_SHAREMODE_SHARED );

   /// Get IMMDeviceEnumerator from COM (CoCreateInstance)
   IMMDeviceEnumerator* deviceEnumerator = NULL;

   hr = CoCreateInstance( __uuidof( MMDeviceEnumerator ), NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS( &deviceEnumerator ) );
   CHECK_HR( "Failed to instantiate the multimedia device enumerator via COM" );

   /// Get the IMMDevice
   _ASSERTE( spDevice == NULL );

   hr = deviceEnumerator->GetDefaultAudioEndpoint( eCapture, eMultimedia, &spDevice );
   CHECK_HR( "Failed to get default audio device" );

   _ASSERTE( spDevice != NULL );

   /// Get the ID from IMMDevice
   hr = spDevice->GetId( &spwstrDeviceId );
   if ( hr != S_OK || spwstrDeviceId == NULL ) {
      LOG_FATAL( "Failed to get the device's ID string" );
      return FALSE;
   }

   LOG_INFO_W( L":  Device ID=%s", spwstrDeviceId );

   /// Get the State from IMMDevice
   hr = spDevice->GetState( &sdwState );
   if ( hr != S_OK || sdwState == NULL ) {
      LOG_FATAL( "Failed to get the device's state" );
      return FALSE;
   }

   if ( sdwState != DEVICE_STATE_ACTIVE ) {
      LOG_FATAL( "The audio device state is not active" );
      return FALSE;
   }

   /// Get the Property Store from IMMDevice
   hr = spDevice->OpenPropertyStore( STGM_READ, &spPropertyStore );
   if ( hr != S_OK || spPropertyStore == NULL ) {
      LOG_FATAL( "Failed to open device property store" );
      return FALSE;
   }

   /// Get the device's properties from the property store
   PropVariantInit( &sDeviceInterfaceFriendlyName );     /// Get the friendly name of the audio adapter for the device
   PropVariantInit( &sDeviceDescription );               /// Get the device's description
   PropVariantInit( &sDeviceFriendlyName );              /// Get the friendly name of the device

   hr = spPropertyStore->GetValue( PKEY_DeviceInterface_FriendlyName, &sDeviceInterfaceFriendlyName );
   if ( hr != S_OK ) {
      LOG_WARN( "Failed to retrieve the friendly name of the audio adapter for the device.  Continuing." );
      sDeviceInterfaceFriendlyName.pcVal = NULL;
   } else {
      LOG_INFO_W( L":  Device audio adapter friendly name=%s", sDeviceInterfaceFriendlyName.pwszVal );
   }

   hr = spPropertyStore->GetValue( PKEY_Device_DeviceDesc, &sDeviceDescription );
   if ( hr != S_OK ) {
      LOG_WARN( "Failed to retrieve the device's description.  Continuing." );
      sDeviceDescription.pcVal = NULL;
   } else {
      LOG_INFO_W( L":  Device description=%s", sDeviceDescription.pwszVal );
   }

   hr = spPropertyStore->GetValue( PKEY_Device_FriendlyName, &sDeviceFriendlyName );
   if ( hr != S_OK ) {
      LOG_WARN( "Failed to retrieve the friendly name of the device.  Continuing." );
      sDeviceFriendlyName.pcVal = NULL;
   } else {
      LOG_INFO_W( L":  Device friendly name=%s", sDeviceFriendlyName.pwszVal );
   }

   /// Use Activate on IMMDevice to create an IAudioClient
   hr = spDevice->Activate( __uuidof( IAudioClient ), CLSCTX_ALL, NULL, (void**) &spAudioClient );
   if ( hr != S_OK || spAudioClient == NULL ) {
      LOG_FATAL( "Failed to create an audio client" );
      return FALSE;
   }

   _ASSERTE( spAudioClient != NULL );

   /// Get the default audio format that the audio driver wants to use
   hr = spAudioClient->GetMixFormat( &spMixFormat );
   if ( hr != S_OK || spMixFormat == NULL ) {
      LOG_FATAL( "Failed to retrieve mix format" );
      return FALSE;
   }

   _ASSERTE( spMixFormat != NULL );

   LOG_DEBUG( "The mix format follows:" );
   audioPrintWaveFormat( spMixFormat );

   hr = spAudioClient->IsFormatSupported( sShareMode, spMixFormat, &spAudioFormatUsed );
   if ( hr == S_OK ) {
      LOG_INFO( "The requested format is supported");
   } else if ( hr == AUDCLNT_E_UNSUPPORTED_FORMAT ) {
      LOG_FATAL( "The requested format is is not supported" );
      return FALSE;
   } else if( hr == S_FALSE && spAudioFormatUsed != NULL) {
      LOG_DEBUG( "The requested format is not available, but this format is..." );
      audioPrintWaveFormat( spAudioFormatUsed );
      return FALSE;
   } else {
      LOG_FATAL( "Failed to validate the requested format" );
      return FALSE;
   }

   /// Determine the audio format
   sAudioFormat = UNKNOWN_AUDIO_FORMAT;

   if ( spMixFormat->wFormatTag == WAVE_FORMAT_PCM && spMixFormat->wBitsPerSample == 8 ) {
      sAudioFormat = PCM_8;
   } else if ( spMixFormat->wFormatTag == WAVE_FORMAT_IEEE_FLOAT && spMixFormat->wBitsPerSample == 32 ) {
      sAudioFormat = IEEE_FLOAT_32;
   } else if ( spMixFormat->wFormatTag == WAVE_FORMAT_EXTENSIBLE ) {
      WAVEFORMATEXTENSIBLE* pFmtEx = (WAVEFORMATEXTENSIBLE*) spMixFormat;

      if ( pFmtEx->SubFormat == KSDATAFORMAT_SUBTYPE_PCM && pFmtEx->Samples.wValidBitsPerSample == 8 ) {
         sAudioFormat = PCM_8;
      } else if ( pFmtEx->SubFormat == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT && pFmtEx->Samples.wValidBitsPerSample == 32 ) {
         sAudioFormat = IEEE_FLOAT_32;
      }
   }

   if ( sAudioFormat == UNKNOWN_AUDIO_FORMAT ) {
      LOG_FATAL( "Failed to match with the audio format" );
      return FALSE;
   }

   _ASSERTE( sAudioFormat != UNKNOWN_AUDIO_FORMAT );

   /// Initialize shared mode audio client
   //  Shared mode streams using event-driven buffering must set both periodicity and bufferDuration to 0.
   hr = spAudioClient->Initialize( sShareMode, AUDCLNT_STREAMFLAGS_EVENTCALLBACK
                                              | AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM, 0, 0, spMixFormat, NULL );
   if ( hr != S_OK ) {
      /// @todo Look at more error codes and print out higher-fidelity error messages
      LOG_FATAL( "Failed to initialize the audio client" );
      return FALSE;
   }

   hr = spAudioClient->GetBufferSize( &suBufferSize );
   CHECK_HR( "Failed to get buffer size" );
   LOG_INFO( "The maximum capacity of the buffer is %" PRIu32" frames or %i ms", suBufferSize, (int) (1.0 / spMixFormat->nSamplesPerSec * 1000 * suBufferSize ) );
   /// Right now, the buffer is ~22ms or about the perfect size to capture
   /// VoIP voice, which is 20ms.

   /// Get the device period
   hr = spAudioClient->GetDevicePeriod( &sDefaultDevicePeriod, &sMinimumDevicePeriod );
   CHECK_HR( "Failed to get audio client device periods" );

   LOG_INFO( "%s:  Default device period=%lli ms", __FUNCTION__, sDefaultDevicePeriod / 10000 );
   LOG_INFO( "%s:  Minimum device period=%lli ms", __FUNCTION__, sMinimumDevicePeriod / 10000 );


   /// Initialize the DTMF buffer
   br = pcmSetQueueSize( spMixFormat->nSamplesPerSec / 1000 * SIZE_OF_QUEUE_IN_MS );
   CHECK_BR( "Failed to allocate PCM queue" );

   LOG_INFO( "%s:  Queue size=%zu bytes or %d ms", __FUNCTION__, gstQueueSize, SIZE_OF_QUEUE_IN_MS );

   /// After everything is initialized, set #gbIsRunning to `true`
   gbIsRunning = true;

   /// Initialize the Goertzel module (and associated threads)
   br = goertzel_init( spMixFormat->nSamplesPerSec );
   CHECK_BR( "Failed to initialioze Goertzel module (and associated threads)" );


   /// Create the callback events
   ghAudioSamplesReadyEvent = CreateEventEx( NULL, NULL, 0, EVENT_MODIFY_STATE | SYNCHRONIZE );
   if ( ghAudioSamplesReadyEvent == NULL ) {
      LOG_FATAL( "Failed to create an audio samples ready event" );
      return FALSE;
   }

   hr = spAudioClient->SetEventHandle( ghAudioSamplesReadyEvent );
   CHECK_HR( "Failed to set audio capture ready event" );

   /// Get the Capture Client
   hr = spAudioClient->GetService( IID_PPV_ARGS( &spCaptureClient ) );
   CHECK_HR( "Failed to get capture client" )

   /// Start the thread
   shCaptureThread = CreateThread( NULL, 0, audioCaptureThread, NULL, 0, NULL );
   if ( shCaptureThread == NULL ) {
      LOG_FATAL( "Failed to create the capture thread" );
      return FALSE;
   }

   /// Start the audio processer
   hr = spAudioClient->Start();
   CHECK_HR( "Failed to start capturing the audio stream" );

   LOG_INFO( "The audio capture interface has been initialized" );

   /// The thread of execution goes back to #wWinMain, which starts the main
   /// message loop
   return TRUE;
}


/// Stop the audio device
/// @return `true` if successful.  `false` if there was a problem.
BOOL audioStopDevice() {
   HRESULT hr;  // HRESULT result

   if ( spAudioClient != NULL ) {
      hr = spAudioClient->Stop();
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

   if ( shCaptureThread != NULL ) {
      br = CloseHandle( shCaptureThread );
      CHECK_BR( "Failed to close hCaptureThread" );
      shCaptureThread = NULL;
   }

   SAFE_RELEASE( spCaptureClient );

   if ( ghAudioSamplesReadyEvent != NULL ) {
      br = CloseHandle( ghAudioSamplesReadyEvent );
      CHECK_BR( "Failed to close gAudioSamplesReadyEvent" );
      ghAudioSamplesReadyEvent = NULL;
   }

   if ( spAudioClient != NULL ) {
      hr = spAudioClient->Reset();
      CHECK_HR( "Failed to release the audio client")
      spAudioClient = NULL;
   }

   pcmReleaseQueue();

   SAFE_RELEASE( spAudioClient );

   hr = PropVariantClear( &sDeviceFriendlyName );
   CHECK_HR( "Failed to release gDeviceFriendlyName" );
   hr = PropVariantClear( &sDeviceDescription );
   CHECK_HR( "Failed to release gDeviceDescription" );
   hr = PropVariantClear( &sDeviceInterfaceFriendlyName );
   CHECK_HR( "Failed to release gDeviceInterfaceFriendlyName" );

   SAFE_RELEASE( spPropertyStore );

   if ( spMixFormat != NULL ) {
      CoTaskMemFree( spMixFormat );
      spMixFormat = NULL;
   }

   if ( spAudioFormatUsed != NULL ) {
      CoTaskMemFree( spAudioFormatUsed );
      spAudioFormatUsed = NULL;
   }

   if ( spwstrDeviceId != NULL ) {
      CoTaskMemFree( spwstrDeviceId );
      spwstrDeviceId = NULL;
   }

   SAFE_RELEASE( spDevice );

   return TRUE;
}
