///////////////////////////////////////////////////////////////////////////////
//          University of Hawaii, College of Engineering
//          DTMF_Decoder22X!Vqrpp1kz9C!ma3mCkbd - EE 469 - Fall 2022
//
/// A Windows Desktop C program that decodes DTMF tones
/// 
/// @file audio.cpp
/// @version 1.0
///
/// @author Mark Nelson <marknels@hawaii.edu>
/// @date   10_Oct_2022
///////////////////////////////////////////////////////////////////////////////

#include "framework.h"
#include <mmdeviceapi.h>
#include <Functiondiscoverykeys_devpkey.h>
#include <strsafe.h>
#include <AudioClient.h>

#include "audio.h"
#include "mvcModel.h"
#include "goertzel.h"
#include "mvcView.h"

#include <assert.h>  // For assert
#include <avrt.h>    // For AvSetMmThreadCharacteristics()
#include <inttypes.h> // For the printf fixed-integer mappings
#include <float.h>    // For FLT_MIN and FLT_MAX

#pragma comment(lib, "avrt")    // Link the MMCSS library
#pragma comment(lib, "rpcrt4.lib")  // TEMP TEMP TEMP



IMMDevice*      gDevice = NULL;
LPWSTR          glpwstrDeviceId = NULL;
DWORD           glpdwState = 0;
IPropertyStore* glpPropertyStore = NULL;
AUDCLNT_SHAREMODE gShareMode = AUDCLNT_SHAREMODE_SHARED;
PROPVARIANT     DeviceInterfaceFriendlyName;  // Container for the friendly name of the audio adapter for the device
PROPVARIANT     DeviceDescription;            // Container for the device's description
PROPVARIANT     DeviceFriendlyName;           // Container for friendly name of the device
//WAVEFORMATEX    audioFormatRequested;
WAVEFORMATEX*   gpMixFormat = NULL;
WAVEFORMATEX*   audioFormatUsed = NULL;
IAudioClient*   glpAudioClient = NULL;
REFERENCE_TIME  gDefaultDevicePeriod = -1;    // Expressed in 100ns units (dev machine = 101,587 = 10.1587ms)
REFERENCE_TIME  gMinimumDevicePeriod = -1;    // Expressed in 100ns units (dev machine = 29,025  = 2.9025ms
UINT32          guBufferSize = 0;             // Dev Machine = 182
HANDLE          hCaptureThread = NULL;
HANDLE          gAudioSamplesReadyEvent = NULL;  // This is externally delcared
IAudioCaptureClient* gCaptureClient = NULL;
// IAudioClockAdjustment* gAudioClockAdjuster = NULL;
static size_t queueSize = 0;  /// Size in bytes of DTMF DFT queue = samplesPerSecond / 1000 * SIZE_OF_QUEUE_IN_MS

CHAR            sBuf[ 256 ];  // Debug buffer   // TODO: put a guard around this
WCHAR           wsBuf[ 256 ];  // Debug buffer   // TODO: put a guard around this


template <class T> void SafeRelease( T** ppT ) {
   if ( *ppT ) {
      ( *ppT )->Release();
      *ppT = NULL;
   }
}

#define MONITOR_INTERVAL_SECONDS (4)   /* Set to 0 to disable monitoring */
UINT64 gFramesToMonitor = 0;           /// set gMonitor when the current frame is > gStartOfMonitor + gFramesToMonitor
UINT64 gStartOfMonitor = UINT64_MAX;
BOOL   gbMonitor = false;     /// Briefly set to 1 to output monitor data

BYTE monitorCh1Max = 0;
BYTE monitorCh1Min = 255;

BOOL isPCM = false;
BOOL isIEEE = false;


DWORD getSamplesPerSecond() {
   return gpMixFormat->nSamplesPerSec;
}


// TODO: Make this generic (good for a variety of formats -- there's a good 
//       example in the git history just before commit 563da34a)
// TODO:  Preprocess this stuff
// NOTE: For now, this code assumes that pData is a 1 channel, 8-bit PCM data stream
BOOL processAudioFrame( BYTE* pData, UINT32 frame, UINT64 framePosition ) {
   assert( pData != NULL );
   assert( isPCM || isIEEE );
   assert( gpMixFormat != NULL );

   BYTE ch1Sample = PCM_8_BIT_SILENCE;

   if ( isIEEE && gpMixFormat->wBitsPerSample == 32 ) {  // IEEE float
      float* fSample = (float*) (pData + ( frame * gpMixFormat->nBlockAlign ));

      INT8 signedSample = (INT8) ( *fSample * (float) PCM_8_BIT_SILENCE );  // This is +127 to -127
      if ( signedSample >= 0 ) {
         ch1Sample = signedSample + PCM_8_BIT_SILENCE;
      } else {
         ch1Sample = PCM_8_BIT_SILENCE + signedSample;  // -1, -2, -3, will turn into 127, 126, 125, ...
      }
   } else if ( isPCM && gpMixFormat->wBitsPerSample == 8 ) {  // 8-bit PCM
      ch1Sample = *(pData + (frame * gpMixFormat->nBlockAlign) );
   } else {
      // Punch out
   }

   if ( gbMonitor )
      ch1Sample = ch1Sample;

   pcmEnqueue( ch1Sample );

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

   return TRUE;
}


// TODO:  Watch the program with Process Monitor and make sure it's not
// over-spinning a thread

/// Collect the audio frames and process them
BOOL captureAudio() {
   HRESULT hr;

   BYTE*  pData;
   UINT32 framesAvailable;
   DWORD  flags;
   UINT64 framePosition;

   assert( gCaptureClient != NULL );

   hr = gCaptureClient->GetBuffer( &pData, &framesAvailable, &flags, &framePosition, NULL );
   if ( hr == S_OK ) {
      // OutputDebugStringA( __FUNCTION__ ":  I got data!" );

      if ( flags == 0 ) {
         // Normal processing
         for ( UINT32 i = 0 ; i < framesAvailable ; i++ ) {
            processAudioFrame( pData, i, framePosition+i );
         }
         
         compute_dtmf_tones_with_goertzel();
          
         if ( hasDtmfTonesChanged ) {
            mvcViewRefreshWindow();
         }
      } 

      if ( flags & AUDCLNT_BUFFERFLAGS_SILENT ) {
         OutputDebugStringA( __FUNCTION__ ":  Buffer flag set:  SILENT" );
         // Nothing so see here.  Move along.
         flags &= !AUDCLNT_BUFFERFLAGS_SILENT;  // Clear AUDCLNT_BUFFERFLAGS_SILENT from flags
      } 
      if ( flags & AUDCLNT_BUFFERFLAGS_DATA_DISCONTINUITY ) {
         OutputDebugStringA( __FUNCTION__ ":  Buffer flag set:  DATA_DISCONTINUITY" );
         // Throw the first packet out
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
      OutputDebugStringA( __FUNCTION__ ":  GetBuffer did not return S_OK.  Investigate!!" );
      isRunning = false;
   }

   return TRUE;
}


DWORD captureThread( LPVOID Context ) {
   OutputDebugStringA( __FUNCTION__ ":  Start capture thread" );

   HRESULT hr;
   HANDLE mmcssHandle = NULL;
   DWORD mmcssTaskIndex = 0;

   /// Initialize COM for the thread
   hr = CoInitializeEx( NULL, COINIT_MULTITHREADED );
   if ( hr != S_OK ) {
      OutputDebugStringA( __FUNCTION__ ":  Failed to initialize COM in thread" );
      ExitThread( -1 );
   }

   /// Set the multimedia class scheduler service, which will set the CPU
   /// priority for this thread
   mmcssHandle = AvSetMmThreadCharacteristics( L"Capture", &mmcssTaskIndex );
   if ( mmcssHandle == NULL ) {
      OutputDebugStringA( __FUNCTION__ ":  Failed to set MMCSS on thread.  Continuing." );
   }
   OutputDebugStringA( __FUNCTION__ ":  Set MMCSS on thread." );

   gFramesToMonitor = MONITOR_INTERVAL_SECONDS * gpMixFormat->nSamplesPerSec;

   /// Audio capture loop
   /// isRunning gets set to false by WM_CLOSE
   
   isRunning = true;

   while ( isRunning ) {
      DWORD dwWaitResult;

      dwWaitResult = WaitForSingleObject( gAudioSamplesReadyEvent, INFINITE );
      if ( dwWaitResult == WAIT_OBJECT_0 ) {
         if ( isRunning ) {
            captureAudio();
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

   /// Cleanup the thread
   CoUninitialize();

   OutputDebugStringA( __FUNCTION__ ":  End capture thread" );

   ExitThread( 0 );
}


/// Print the WAVEFORMATEX or WAVEFORMATEXTENSIBLE structure to OutputDebug
BOOL printAudioFormat( WAVEFORMATEX* pFmt ) {
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


BOOL initAudioDevice( HWND hWnd ) {
   HRESULT hr;  /// Result handle used by just about all Windows API calls

   if ( gShareMode == AUDCLNT_SHAREMODE_EXCLUSIVE ) {
      OutputDebugStringA( __FUNCTION__ ":  Exclusive mode not supported right now" );
      return FALSE;
   }

   /// Get IMMDeviceEnumerator from COM (CoCreateInstance)
   IMMDeviceEnumerator* deviceEnumerator = NULL;

   hr = CoCreateInstance( __uuidof( MMDeviceEnumerator ), NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS( &deviceEnumerator ) );
   if ( hr != S_OK ) {
      OutputDebugStringA( __FUNCTION__ ":  Failed to instantiate the multimedia device enumerator via COM" );
      return FALSE;
   }

   /// Get the IMMDevice
   assert( gDevice == NULL );

   hr = deviceEnumerator->GetDefaultAudioEndpoint( eCapture, eMultimedia, &gDevice );
   if ( hr != S_OK || gDevice == NULL ) {
      OutputDebugStringA( __FUNCTION__ ":  Failed to get default audio device" );
      return FALSE;
   }

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
   PropVariantInit( &DeviceInterfaceFriendlyName );     /// Get the friendly name of the audio adapter for the device
   PropVariantInit( &DeviceDescription );               /// Get the device's description
   PropVariantInit( &DeviceFriendlyName );              /// Get the friendly name of the device

   hr = glpPropertyStore->GetValue( PKEY_DeviceInterface_FriendlyName, &DeviceInterfaceFriendlyName );
   if ( hr != S_OK ) {
      OutputDebugStringA( __FUNCTION__ ":  Failed to retrieve the friendly name of the audio adapter for the device.  Continuing." );
      DeviceInterfaceFriendlyName.pcVal = NULL;
   } else {
      swprintf_s( wsBuf, sizeof( wsBuf ), __FUNCTIONW__ L":  Device audio adapter friendly name=%s", DeviceInterfaceFriendlyName.pwszVal );
      OutputDebugStringW( wsBuf );
   }

   hr = glpPropertyStore->GetValue( PKEY_Device_DeviceDesc, &DeviceDescription );
   if ( hr != S_OK ) {
      OutputDebugStringA( __FUNCTION__ ":  Failed to retrieve the device's description.  Continuing." );
      DeviceDescription.pcVal = NULL;
   } else {
      swprintf_s( wsBuf, sizeof( wsBuf ), __FUNCTIONW__ L":  Device description=%s", DeviceDescription.pwszVal );
      OutputDebugStringW( wsBuf );
   }

   hr = glpPropertyStore->GetValue( PKEY_Device_FriendlyName, &DeviceFriendlyName );
   if ( hr != S_OK ) {
      OutputDebugStringA( __FUNCTION__ ":  Failed to retrieve the friendly name of the device.  Continuing." );
      DeviceFriendlyName.pcVal = NULL;
   } else {
      swprintf_s( wsBuf, sizeof( wsBuf ), __FUNCTIONW__ L":  Device friendly name=%s", DeviceFriendlyName.pwszVal );
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
   printAudioFormat( gpMixFormat );

   hr = glpAudioClient->IsFormatSupported( gShareMode, gpMixFormat, &audioFormatUsed );
   if ( hr == S_OK ) {
      OutputDebugStringA( __FUNCTION__ ":  The requested format is supported");
   } else if ( hr == AUDCLNT_E_UNSUPPORTED_FORMAT ) {
      OutputDebugStringA( __FUNCTION__ ":  The requested format is is not supported" );
      return FALSE;
   } else if( hr == S_FALSE && audioFormatUsed != NULL) {
      OutputDebugStringA( __FUNCTION__ ":  The requested format is not available, but this format is..." );
      printAudioFormat( audioFormatUsed );
      return FALSE;
   } else {
      OutputDebugStringA( __FUNCTION__ ":  Failed to validate the requested format" );
      return FALSE;
   }

   if ( gpMixFormat->wFormatTag == WAVE_FORMAT_PCM ) {
      isPCM = true;
   } else if ( gpMixFormat->wFormatTag == WAVE_FORMAT_IEEE_FLOAT ) {
      isIEEE = true;
   } else if ( gpMixFormat->wFormatTag == WAVE_FORMAT_EXTENSIBLE ) {
      WAVEFORMATEXTENSIBLE* pFmtEx = (WAVEFORMATEXTENSIBLE*) gpMixFormat;

      if ( pFmtEx->SubFormat == KSDATAFORMAT_SUBTYPE_PCM ) {
         isPCM = true;
      } else if ( pFmtEx->SubFormat == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT ) {
         isIEEE = true;
      } else {
         OutputDebugStringA( __FUNCTION__ ":  Unknown extended audio format" );
         return FALSE;
      }
   } else {
      OutputDebugStringA( __FUNCTION__ ":  Unknown audio format" );
      return FALSE;
   }

   assert( isPCM || isIEEE );

   /// Initialize shared mode audio client
   //  Shared mode streams using event-driven buffering must set both periodicity and bufferDuration to 0.
   hr = glpAudioClient->Initialize( gShareMode, AUDCLNT_STREAMFLAGS_EVENTCALLBACK
                                              | AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM
                                                                                  , 0, 0, gpMixFormat, NULL );
   if ( hr != S_OK ) {
      OutputDebugStringA( __FUNCTION__ ":  Failed to initialize the audio client" );
      return FALSE;
   }
   // TODO: Look at the error code and print out higher-fidelity error message

   // OutputDebugStringA( __FUNCTION__ ":  The audio client has been initialized" );

   hr = glpAudioClient->GetBufferSize( &guBufferSize );
   if ( hr != S_OK ) {
      OutputDebugStringA( __FUNCTION__ ":  Failed to get buffer size" );
      return FALSE;
   }

   /// Get the device period
   hr = glpAudioClient->GetDevicePeriod( &gDefaultDevicePeriod, &gMinimumDevicePeriod );
   if ( hr != S_OK ) {
      OutputDebugStringA( __FUNCTION__ ":  Failed to get audio client device periods" );
      return FALSE;
   }

   sprintf_s( sBuf, sizeof( sBuf ), "%s:  Default device period=%lli ms", __FUNCTION__, gDefaultDevicePeriod/10000 );
   OutputDebugStringA( sBuf );

   sprintf_s( sBuf, sizeof( sBuf ), "%s:  Minimum device period=%lli ms", __FUNCTION__, gMinimumDevicePeriod/10000 );
   OutputDebugStringA( sBuf );


   /// Initialize the DTMF buffer
   queueSize = gpMixFormat->nSamplesPerSec / 1000 * SIZE_OF_QUEUE_IN_MS;
   if ( pcmSetQueueSize( queueSize ) != TRUE ) {
      OutputDebugStringA( __FUNCTION__ ":  Failed to allocate PCM queue" );
      return FALSE;
   }

   sprintf_s( sBuf, sizeof( sBuf ), "%s:  Queue size=%zu", __FUNCTION__, queueSize );
   OutputDebugStringA( sBuf );

   /// Initialize the Goertzel module
   if ( goertxzel_init( queueSize, gpMixFormat->nSamplesPerSec ) != TRUE ) {
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
   if ( hr != S_OK ) {
      OutputDebugStringA( __FUNCTION__ ":  Failed to set audio capture ready event" );
      return FALSE;
   }

   /// Get the Capture Client
   hr = glpAudioClient->GetService( IID_PPV_ARGS( &gCaptureClient ) );
   if ( hr != S_OK ) {
      OutputDebugStringA( __FUNCTION__ ":  Failed to get capture client" );
      return FALSE;
   }

   /// Start the thread
   hCaptureThread = CreateThread( NULL, 0, captureThread, NULL, 0, NULL );
   if ( hCaptureThread == NULL ) {
      OutputDebugStringA( __FUNCTION__ ":  Failed to create the capture thread" );
      return FALSE;
   }

   /// Start the audio processer
   hr = glpAudioClient->Start();
   if ( hr != S_OK ) {
      OutputDebugStringA( __FUNCTION__ ":  Failed to start capturing the audio stream" );
      return FALSE;
   }

   OutputDebugStringA( __FUNCTION__ ":  The audio capture interface has been initialized" );

   /// The thread of execution goes back to wWinMain, which starts the main message loop
   return TRUE;
}


BOOL stopAudioDevice( HWND ) {
   HRESULT hr;

   if ( glpAudioClient != NULL ) {
      hr = glpAudioClient->Stop();
      if ( hr != S_OK ) {
         OutputDebugStringA( __FUNCTION__ ":  Stopping the audio stream returned an odd value.  Investigate!!" );
         return FALSE;
      }
   }

   return TRUE;
}


BOOL cleanupAudioDevice() {

   if ( hCaptureThread != NULL ) {
      CloseHandle( hCaptureThread );
      hCaptureThread = NULL;
   }

   SafeRelease( &gCaptureClient );

   if ( gAudioSamplesReadyEvent != NULL ) {
      CloseHandle( gAudioSamplesReadyEvent );
      gAudioSamplesReadyEvent = NULL;
   }

   if ( glpAudioClient != NULL ) {
      glpAudioClient->Reset();
      glpAudioClient = NULL;
   }

   pcmReleaseQueue();

   SafeRelease( &glpAudioClient );

   PropVariantClear( &DeviceFriendlyName );
   PropVariantClear( &DeviceDescription );
   PropVariantClear( &DeviceInterfaceFriendlyName );

   SafeRelease( &glpPropertyStore );

   if ( gpMixFormat != NULL ) {
      CoTaskMemFree( gpMixFormat );
      gpMixFormat = NULL;
   }

   if ( audioFormatUsed != NULL ) {
      CoTaskMemFree( audioFormatUsed );
      audioFormatUsed = NULL;
   }

   if ( glpwstrDeviceId != NULL ) {
      CoTaskMemFree( glpwstrDeviceId );
      glpwstrDeviceId = NULL;
   }

   SafeRelease( &gDevice );

   return TRUE;
}
