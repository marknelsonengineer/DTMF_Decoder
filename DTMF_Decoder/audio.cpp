///////////////////////////////////////////////////////////////////////////////
//          University of Hawaii, College of Engineering
//          DTMF_Decoder - EE 469 - Fall 2022
//
//  A Windows Desktop C program that decodes DTMF tones
//
/// Initialize and process audio from the default Windows audio capture device
///
/// @see https://learn.microsoft.com/en-us/windows/win32/api/_coreaudio/
/// @see https://matthewvaneerde.wordpress.com/2017/10/17/how-to-negotiate-an-audio-format-for-a-windows-audio-session-api-wasapi-client/
///
/// ## Audio API
/// | API                                            | Link                                                                                                                           |
/// |------------------------------------------------|--------------------------------------------------------------------------------------------------------------------------------|
/// | `IMMDevice`                                    | https://learn.microsoft.com/en-us/windows/win32/api/mmdeviceapi/nn-mmdeviceapi-immdevice                                       |
/// | `IAudioClient`                                 | https://learn.microsoft.com/en-us/windows/win32/api/audioclient/nn-audioclient-iaudioclient                                    |
/// | `IAudioCaptureClient`                          | https://learn.microsoft.com/en-us/windows/win32/api/audioclient/nn-audioclient-iaudiocaptureclient                             |
/// | `WAVEFORMATEX`                                 | https://learn.microsoft.com/en-us/windows/win32/api/mmeapi/ns-mmeapi-waveformatex                                              |
/// | `WAVEFORMATEXTENSIBLE`                         | https://learn.microsoft.com/en-us/windows/win32/api/mmreg/ns-mmreg-waveformatextensible                                        |
/// | `IPropertyStore`                               | https://learn.microsoft.com/en-us/windows/win32/api/propsys/nn-propsys-ipropertystore                                          |
/// | `IMMDeviceEnumerator::GetDefaultAudioEndpoint` | https://learn.microsoft.com/en-us/windows/win32/api/mmdeviceapi/nf-mmdeviceapi-immdeviceenumerator-getdefaultaudioendpoint     |
/// | `IMMDevice::GetId`                             | https://learn.microsoft.com/en-us/windows/win32/api/mmdeviceapi/nf-mmdeviceapi-immdevice-getid                                 |
/// | `IMMDevice::GetState`                          | https://learn.microsoft.com/en-us/windows/win32/api/mmdeviceapi/nf-mmdeviceapi-immdevice-getstate                              |
/// | `IMMDevice::OpenPropertyStore`                 | https://learn.microsoft.com/en-us/windows/win32/api/mmdeviceapi/nf-mmdeviceapi-immdevice-openpropertystore                     |
/// | `PropVariantInit`                              | https://learn.microsoft.com/en-us/windows/win32/api/propidl/nf-propidl-propvariantinit                                         |
/// | `IPropertyStore::GetValue`                     | https://learn.microsoft.com/en-us/previous-versions/windows/desktop/legacy/bb761473(v=vs.85)                                   |
/// | `IAudioClient::GetMixFormat`                   | https://learn.microsoft.com/en-us/windows/win32/api/audioclient/nf-audioclient-iaudioclient-getmixformat                       |
/// | `IAudioClient::IsFormatSupported`              | https://learn.microsoft.com/en-us/windows/win32/api/audioclient/nf-audioclient-iaudioclient-isformatsupported                  |
/// | `IAudioClient::Initialize`                     | https://learn.microsoft.com/en-us/windows/win32/api/audioclient/nf-audioclient-iaudioclient-initialize                         |
/// | `IMMDevice::Activate`                          | https://learn.microsoft.com/en-us/windows/win32/api/mmdeviceapi/nf-mmdeviceapi-immdevice-activate                              |
/// | `IAudioClient::GetBufferSize`                  | https://learn.microsoft.com/en-us/windows/win32/api/audioclient/nf-audioclient-iaudioclient-getbuffersize                      |
/// | `IAudioClient::GetDevicePeriod`                | https://learn.microsoft.com/en-us/windows/win32/api/audioclient/nf-audioclient-iaudioclient-getdeviceperiod                    |
/// | `IAudioClient::GetService`                     | https://learn.microsoft.com/en-us/windows/win32/api/audioclient/nf-audioclient-iaudioclient-getservice                         |
/// | `IAudioClient::SetEventHandle`                 | https://learn.microsoft.com/en-us/windows/win32/api/audioclient/nf-audioclient-iaudioclient-seteventhandle                     |
/// | `IAudioClient::Start`                          | https://learn.microsoft.com/en-us/windows/win32/api/audioclient/nf-audioclient-iaudioclient-start                              |
/// | `AvSetMmThreadCharacteristics`                 | https://learn.microsoft.com/en-us/windows/win32/api/avrt/nf-avrt-avsetmmthreadcharacteristicsa                                 |
/// | `AvRevertMmThreadCharacteristics`              | https://learn.microsoft.com/en-us/windows/win32/api/avrt/nf-avrt-avrevertmmthreadcharacteristics                               |
/// | `IAudioCaptureClient::GetBuffer`               | https://learn.microsoft.com/en-us/windows/win32/api/audioclient/nf-audioclient-iaudiocaptureclient-getbuffer                   |
/// | `IAudioCaptureClient::ReleaseBuffer`           | https://learn.microsoft.com/en-us/windows/win32/api/audioclient/nf-audioclient-iaudiocaptureclient-releasebuffer               |
/// | `IAudioClient::Stop`                           | https://learn.microsoft.com/en-us/windows/win32/api/audioclient/nf-audioclient-iaudioclient-stop                               |
/// | `IAudioClient::Reset`                          | https://learn.microsoft.com/en-us/windows/win32/api/audioclient/nf-audioclient-iaudioclient-reset                              |
///
/// ## COM API
/// | API                 | Link                                                                                          |
/// |---------------------|-----------------------------------------------------------------------------------------------|
/// | `CoInitializeEx`    | https://learn.microsoft.com/en-us/windows/win32/api/combaseapi/nf-combaseapi-coinitializeex   |
/// | `CoUninitialize`    | https://learn.microsoft.com/en-us/windows/win32/api/combaseapi/nf-combaseapi-couninitialize   |
/// | `CoCreateInstance`  | https://learn.microsoft.com/en-us/windows/win32/api/combaseapi/nf-combaseapi-cocreateinstance |
/// | `IUnknown::Release` | https://learn.microsoft.com/en-us/windows/win32/api/unknwn/nf-unknwn-iunknown-release         |
/// | `PropVariantClear`  | https://learn.microsoft.com/en-us/windows/win32/api/combaseapi/nf-combaseapi-propvariantclear |
/// | `CoTaskMemFree`     | https://learn.microsoft.com/en-us/windows/win32/api/combaseapi/nf-combaseapi-cotaskmemfree    |
///
/// ## Threads & Aynchronization API
/// | API                      | Link                                                                                                    |
/// |--------------------------|---------------------------------------------------------------------------------------------------------|
/// | `CreateThread`           | https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-createthread |
/// | `CreateEventExW`         | https://learn.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-createeventexw                 |
/// | `ThreadProc`             | https://learn.microsoft.com/en-us/previous-versions/windows/desktop/legacy/ms686736(v=vs.85)            |
/// | `WaitForSingleObject`    | https://learn.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-waitforsingleobject            |
/// | `ExitThread`             | https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-exitthread   |
///
/// ## Generic Win32 API
/// | API                    | Link                                                                                         |
/// |------------------------|----------------------------------------------------------------------------------------------|
/// | `CloseHandle`          | https://learn.microsoft.com/en-us/windows/win32/api/handleapi/nf-handleapi-closehandle       |
///
/// @todo Watch the program with Process Monitor and make sure it's not
///       over-spinning any threads.  So far, it looks very good.
///
/// @file    audio.cpp
/// @author  Mark Nelson <marknels@hawaii.edu>
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
   #define MONITOR_INTERVAL_SECONDS (4)          /**< The monitoring interval.  Set to 0 to disable monitoring */
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
/// Inlined for performance.
///
/// @param pData      Pointer to the head of the audio bufer
/// @param frameIndex The frameIndex number to process
/// @return `TRUE` if successful.  `FALSE` if there was a problem.
__forceinline static BOOL processAudioFrame(
   _In_     const BYTE*    pData,
   _In_     const UINT32   frameIndex ) {

   _ASSERTE( pData != NULL );
   _ASSERTE( sAudioFormat != UNKNOWN_AUDIO_FORMAT );
   _ASSERTE( spMixFormat != NULL );

   BYTE ch1Sample = PCM_8_BIT_SILENCE;

   switch ( sAudioFormat ) {
      case IEEE_FLOAT_32: {
            float* fSample = (float*) ( pData + ( (size_t) frameIndex * spMixFormat->nBlockAlign ) );  // This is from +1 to -1

            INT8 signedSample = (INT8) ( *fSample * (float) PCM_8_BIT_SILENCE );  // This is +127 to -127
            if ( signedSample >= 0 ) {
               ch1Sample = signedSample + PCM_8_BIT_SILENCE;
            } else {
               ch1Sample = PCM_8_BIT_SILENCE + signedSample;  // -1, -2, -3, will turn into 127, 126, 125, ...
            }
            break;
         }
      case PCM_8:
         ch1Sample = *( pData + ( (size_t) frameIndex * spMixFormat->nBlockAlign ) );
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
/// On virtualized systems, the hypervisor can play Merry Hell with realtime
/// applications like this.  Windows has several features to ensure that
/// realtime applications (like audio capture) have high proitory, but the
/// hypervisor doesn't have visibility to that.
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
/// Inlined for performance.
///
/// It's normal for the first buffer to have DATA_DISCONTINUITY set
__forceinline static void audioCapture() {
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
         CHECK_BR_Q( IDS_AUDIO_FAILED_TO_COMPUTE_DTMF_TONES, 0 );  // "Failed to compute DTMF tones.  Exiting.  Investigate!"
      }

      /// Carefully analyze the flags returned by GetBuffer
      if ( flags & AUDCLNT_BUFFERFLAGS_SILENT ) {
         LOG_INFO_R( IDS_AUDIO_BUFFER_SILENT );  // "Buffer flag set: SILENT"
         // Nothing so see here.  Move along.
         flags &= ~AUDCLNT_BUFFERFLAGS_SILENT;  // Clear AUDCLNT_BUFFERFLAGS_SILENT from flags
      }
      if ( flags & AUDCLNT_BUFFERFLAGS_DATA_DISCONTINUITY ) {
         LOG_INFO_R( IDS_AUDIO_BUFFER_DISCONTINUOUS );  // "Buffer flag set: DATA_DISCONTINUITY"
         // Throw these packets out
         flags &= ~AUDCLNT_BUFFERFLAGS_DATA_DISCONTINUITY;  // Clear AUDCLNT_BUFFERFLAGS_DATA_DISCONTINUITY from flags
      }
      if ( flags & AUDCLNT_BUFFERFLAGS_TIMESTAMP_ERROR ) {
         LOG_INFO_R( IDS_AUDIO_BUFFER_TIMESTAMP_MISALIGNED );  // "Buffer flag set: TIMESTAMP_ERROR"
         // Throw these packets out
         flags &= ~AUDCLNT_BUFFERFLAGS_TIMESTAMP_ERROR;  // Clear AUDCLNT_BUFFERFLAGS_TIMESTAMP_ERROR from flags
      }
      if ( flags != 0 ) {
         QUEUE_FATAL( IDS_AUDIO_BUFFER_OTHER_ISSUE );  // "Some other bufer flags are set.  Investigate!"
         // Throw these packets out
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
            QUEUE_FATAL( IDS_AUDIO_FAILED_TO_RELEASE_AUDIO_BUFFER );  // "ReleaseBuffer didn't return S_OK.  Exiting.  Investigate!"
         }
      }

   } else if ( hr == AUDCLNT_S_BUFFER_EMPTY ) {
      LOG_INFO_R( IDS_AUDIO_GETBUFFER_EMPTY );  // "GetBuffer returned an empty buffer.  Continue."
   } else if ( hr == AUDCLNT_E_OUT_OF_ORDER ) {
      LOG_INFO_R( IDS_AUDIO_GETBUFFER_NOT_SEQUENTIAL );  // "GetBuffer returned out of order data.  Continue."
   } else {
      /// If the audio device changes (unplugged, for example) then GetBuffer
      /// will return something unexpected and we should see it here.  If this
      /// happens, gracefully shutdown the app.

      QUEUE_FATAL( IDS_AUDIO_GETBUFFER_NOT_OK );  // "GetBuffer did not return S_OK.  Exiting.  Investigate!"
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
   LOG_TRACE_R( IDS_AUDIO_START_THREAD );  // "Start capture thread"

   HRESULT hr;                  // HRESULT result
   HANDLE  mmcssHandle = NULL;

   /// Initialize COM for the thread
   hr = CoInitializeEx( NULL, COINIT_MULTITHREADED );
   if ( hr != S_OK ) {
      QUEUE_FATAL( IDS_DTMF_DECODER_FAILED_TO_INITIALIZE_COM );  // "Failed to initialize COM."
      ExitThread( 0xFFFF );
   }

   /// Set the multimedia class scheduler service, which will set the CPU
   /// priority for this thread
   mmcssHandle = AvSetMmThreadCharacteristicsW( L"Capture", &gdwMmcssTaskIndex );
   if ( mmcssHandle == NULL ) {
      LOG_INFO_R( IDS_AUDIO_FAILED_TO_SET_MMCSS );  // "Failed to set MMCSS on the audio capture thread.  Continuing."
   } else {
      LOG_TRACE_R( IDS_AUDIO_SET_MMCSS );  // "Set MMCSS on the audio capture thread."
   }

   #ifdef MONITOR_PCM_AUDIO
      suFramesToMonitor = (UINT64) MONITOR_INTERVAL_SECONDS * spMixFormat->nSamplesPerSec;
   #endif

   /// Audio capture loop
   while ( gbIsRunning ) {
      DWORD dwWaitResult;

      dwWaitResult = WaitForSingleObject( ghAudioSamplesReadyEvent, INFINITE );
      if ( dwWaitResult == WAIT_OBJECT_0 ) {
         if ( gbIsRunning ) {
            audioCapture();
         }
      } else {
         QUEUE_FATAL( IDS_AUDIO_WAIT_FAILED );  // "WaitForSingleObject in audio capture thread failed.  Exiting.  Investigate!"
         break;  // While loop
      }
   }

   // Done.  Time to cleanup the thread

   if ( mmcssHandle != NULL ) {
      if ( !AvRevertMmThreadCharacteristics( mmcssHandle ) ) {
         LOG_INFO_R( IDS_AUDIO_FAILED_TO_REVERT_MMCSS );  // "Failed to revert MMCSS on the audio capture thread.  Continuing."
      }
      mmcssHandle = NULL;
   }

   CoUninitialize();

   LOG_TRACE_R( IDS_AUDIO_END_THREAD );  // "End audio capture thread"

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
static BOOL audioPrintWaveFormat( _In_ const WAVEFORMATEX* pFmt ) {
   _ASSERTE( pFmt != NULL );

   if ( pFmt->wFormatTag == WAVE_FORMAT_EXTENSIBLE ) {
      LOG_DEBUG_R( IDS_AUDIO_USING_WAVE_FORMAT_EXTENSIBLE );  // "Using WAVE_FORMAT_EXTENSIBLE format"
      WAVEFORMATEXTENSIBLE* const pFmtEx = (WAVEFORMATEXTENSIBLE*) pFmt;

      LOG_DEBUG_R( IDS_AUDIO_FORMAT_CHANNELS,              pFmtEx->Format.nChannels );             // "Channels=%" PRIu16
      LOG_DEBUG_R( IDS_AUDIO_FORMAT_SAMPLES_PER_SECOND,    pFmtEx->Format.nSamplesPerSec );        // "Samples per Second=%" PRIu32
      LOG_DEBUG_R( IDS_AUDIO_FORMAT_BYTES_PER_SECOND,      pFmtEx->Format.nAvgBytesPerSec );       // "Bytes per Second=%" PRIu32
      LOG_DEBUG_R( IDS_AUDIO_FORMAT_FRAME_ALIGNMENT,       pFmtEx->Format.nBlockAlign );           // "Block (frame) alignment, in bytes=%" PRIu32
      LOG_DEBUG_R( IDS_AUDIO_FORMAT_BITS_PER_SAMPLE,       pFmtEx->Format.wBitsPerSample );        // "Bits per sample=%" PRIu32
      LOG_DEBUG_R( IDS_AUDIO_FORMAT_VALID_BITS_PER_SAMPLE, pFmtEx->Samples.wValidBitsPerSample );  // "Valid bits per sample=%" PRIu16

      if ( pFmtEx->SubFormat == KSDATAFORMAT_SUBTYPE_PCM ) {
         LOG_DEBUG_R( IDS_AUDIO_EXTENDED_FORMAT_PCM );      // "Extended wave format is PCM"
      } else if ( pFmtEx->SubFormat == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT ) {
         LOG_DEBUG_R( IDS_AUDIO_EXTENDED_FORMAT_FLOAT );    // "Extended wave format is IEEE Float"
      } else {
         LOG_DEBUG_R( IDS_AUDIO_EXTENDED_FORMAT_UNKNOWN );  // "Extended wave format is not recognized"
      }
   } else {
      LOG_DEBUG_R( IDS_AUDIO_USING_WAVE_FORMAT );  // "Using WAVE_FORMAT format"

      if ( pFmt->wFormatTag == WAVE_FORMAT_PCM ) {
         LOG_DEBUG_R( IDS_AUDIO_FORMAT_PCM );      // "Wave format is PCM"
      } else if ( pFmt->wFormatTag == WAVE_FORMAT_IEEE_FLOAT ) {
         LOG_DEBUG_R( IDS_AUDIO_FORMAT_FLOAT );    // "Wave format is IEEE Float"
      } else {
         LOG_DEBUG_R( IDS_AUDIO_FORMAT_UNKNOWN );  // "Wave format is not recognized"
      }

      LOG_DEBUG_R( IDS_AUDIO_FORMAT_CHANNELS,           pFmt->nChannels );        // "Channels=%" PRIu16
      LOG_DEBUG_R( IDS_AUDIO_FORMAT_SAMPLES_PER_SECOND, pFmt->nSamplesPerSec );   // "Samples per Second=%" PRIu32
      LOG_DEBUG_R( IDS_AUDIO_FORMAT_BYTES_PER_SECOND,   pFmt->nAvgBytesPerSec );  // "Bytes per Second=%" PRIu32
      LOG_DEBUG_R( IDS_AUDIO_FORMAT_FRAME_ALIGNMENT,    pFmt->nBlockAlign );      // "Block (frame) alignment, in bytes=%" PRIu32
      LOG_DEBUG_R( IDS_AUDIO_FORMAT_BITS_PER_SAMPLE,    pFmt->wBitsPerSample );   // "Bits per sample=%" PRIu32
   }

   return TRUE;
}


/// Initialize the audio capture device
///
/// @return `TRUE` if successful.  `FALSE` if there was a problem.
BOOL audioInit() {
   if ( sShareMode == AUDCLNT_SHAREMODE_EXCLUSIVE ) {
      RETURN_FATAL( IDS_AUDIO_EXCLUSIVE_MODE_UNSUPPORTED );  // "Exclusive mode not supported right now.  Exiting."
   }

   _ASSERTE( sShareMode == AUDCLNT_SHAREMODE_SHARED );

   /// Create the callback events
   ghAudioSamplesReadyEvent = CreateEventExW(
      NULL,                                // Default security attributes
      NULL,                                // Object name
      0,                                   // Configuration flags
      EVENT_MODIFY_STATE | SYNCHRONIZE );  // Desired access
   if ( ghAudioSamplesReadyEvent == NULL ) {
      RETURN_FATAL( IDS_AUDIO_FAILED_TO_CREATE_READY_EVENT );  // "Failed to create an audio samples ready event"
   }


   LOG_INFO_R( IDS_AUDIO_INIT_SUCCESSFUL );  // "The audio capture interface has been initialized"

   /// The thread of execution goes back to #wWinMain, which starts the main
   /// message loop
   return TRUE;
}


/// Start the audio capture thread
///
/// @return `TRUE` if successful.  `FALSE` if there was a problem.
BOOL audioStart() {
   HRESULT hr;  // HRESULT result
   BOOL    br;  // BOOL result

   _ASSERTE( ghMainWindow != NULL );
   _ASSERTE( ghMainMenu != NULL );
   _ASSERTE( ghAudioSamplesReadyEvent != NULL );

   /// Disable the `Start Capture` menu item
   br = EnableMenuItem( ghMainMenu, IDM_AUDIO_STARTCAPTURE, MF_DISABLED );
   if ( br == -1 ) {
      RETURN_FATAL( IDS_AUDIO_FAILED_TO_SET_MENU_STATE );  // "Failed to set menu state.  Exiting."
   }


   /// Get IMMDeviceEnumerator from COM (CoCreateInstance)
   IMMDeviceEnumerator* deviceEnumerator = NULL;

   hr = CoCreateInstance( __uuidof( MMDeviceEnumerator ), NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS( &deviceEnumerator ) );
   CHECK_HR_R( IDS_AUDIO_FAILED_TO_CREATE_DEVICE );  // "Failed to instantiate the multimedia device enumerator via COM.  Exiting."

   /// Get the IMMDevice
   _ASSERTE( spDevice == NULL );

   hr = deviceEnumerator->GetDefaultAudioEndpoint( eCapture, eMultimedia, &spDevice );
   CHECK_HR_R( IDS_AUDIO_FAILED_TO_GET_DEFAULT_DEVICE );  // "Failed to get default audio device"

   _ASSERTE( spDevice != NULL );

   /// Get the ID from IMMDevice
   hr = spDevice->GetId( &spwstrDeviceId );
   if ( hr != S_OK || spwstrDeviceId == NULL ) {
      RETURN_FATAL( IDS_AUDIO_FAILED_TO_GET_DEVICE_ID );  // "Failed to get the audio device's ID string"
   }

   LOG_INFO_R( IDS_AUDIO_DEVICE_ID, spwstrDeviceId );  // "Device ID = %s"

   /// Get the State from IMMDevice
   hr = spDevice->GetState( &sdwState );
   if ( hr != S_OK || sdwState == NULL ) {
      RETURN_FATAL( IDS_AUDIO_FAILED_TO_GET_DEVICE_STATE );  // "Failed to get the audio device's state"
   }

   if ( sdwState != DEVICE_STATE_ACTIVE ) {
      RETURN_FATAL( IDS_AUDIO_DEVICE_NOT_ACTIVE );  // "The audio device state is not active"
   }

   /// Get the Property Store from IMMDevice
   hr = spDevice->OpenPropertyStore( STGM_READ, &spPropertyStore );
   if ( hr != S_OK || spPropertyStore == NULL ) {
      RETURN_FATAL( IDS_AUDIO_FAILED_TO_OPEN_PROPERTIES );  // "Failed to open device property store"
   }

   /// Get the device's properties from the property store
   PropVariantInit( &sDeviceInterfaceFriendlyName );     /// Get the friendly name of the audio adapter for the device
   PropVariantInit( &sDeviceDescription );               /// Get the device's description
   PropVariantInit( &sDeviceFriendlyName );              /// Get the friendly name of the device

   hr = spPropertyStore->GetValue( PKEY_DeviceInterface_FriendlyName, &sDeviceInterfaceFriendlyName );
   if ( hr != S_OK ) {
      LOG_WARN_R( IDS_AUDIO_FAILED_TO_RETRIEVE_PROPERTY, L"Device Interface Friendly Name" );  // "Failed to retrieve the [%s] property from the audio driver for this device.  Continuing."
      sDeviceInterfaceFriendlyName.pcVal = NULL;
   } else {
      LOG_INFO_R( IDS_AUDIO_DEVICE_INTERFACE_NAME, sDeviceInterfaceFriendlyName.pwszVal );  // "Device interface friendly name=%s"
   }

   hr = spPropertyStore->GetValue( PKEY_Device_DeviceDesc, &sDeviceDescription );
   if ( hr != S_OK ) {
      LOG_WARN_R( IDS_AUDIO_FAILED_TO_RETRIEVE_PROPERTY, L"Device Description" );  // "Failed to retrieve the [%s] property from the audio driver for this device.  Continuing."
      sDeviceDescription.pcVal = NULL;
   } else {
      LOG_INFO_R( IDS_AUDIO_DEVICE_DESCRIPTION, sDeviceDescription.pwszVal );  // "Device description=%s"
   }

   hr = spPropertyStore->GetValue( PKEY_Device_FriendlyName, &sDeviceFriendlyName );
   if ( hr != S_OK ) {
      LOG_WARN_R( IDS_AUDIO_FAILED_TO_RETRIEVE_PROPERTY, L"Device Friendly Name" );  // "Failed to retrieve the [%s] property from the audio driver for this device.  Continuing."
      sDeviceFriendlyName.pcVal = NULL;
   } else {
      LOG_INFO_R( IDS_AUDIO_DEVICE_NAME, sDeviceFriendlyName.pwszVal );  // "Device friendly name=%s"
   }

   /// Use Activate on IMMDevice to create an IAudioClient
   hr = spDevice->Activate( __uuidof( IAudioClient ), CLSCTX_ALL, NULL, (void**) &spAudioClient );
   if ( hr != S_OK || spAudioClient == NULL ) {
      RETURN_FATAL( IDS_AUDIO_FAILED_TO_ACTIVATE );  // "Failed to activate an audio client"
   }

   _ASSERTE( spAudioClient != NULL );

   /// Get the default audio format that the audio driver wants to use
   hr = spAudioClient->GetMixFormat( &spMixFormat );
   if ( hr != S_OK || spMixFormat == NULL ) {
      RETURN_FATAL( IDS_AUDIO_FAILED_TO_GET_MIX_FORMAT );  // "Failed to retrieve mix format"
   }

   _ASSERTE( spMixFormat != NULL );

   LOG_DEBUG_R( IDS_AUDIO_MIX_FORMAT );  // "The mix format follows:"
   audioPrintWaveFormat( spMixFormat );

   hr = spAudioClient->IsFormatSupported( sShareMode, spMixFormat, &spAudioFormatUsed );
   if ( hr == S_OK ) {
      LOG_INFO_R( IDS_AUDIO_FORMAT_SUPPORTED );  // "The requested format is supported"
   } else if ( hr == AUDCLNT_E_UNSUPPORTED_FORMAT ) {
      RETURN_FATAL( IDS_AUDIO_FORMAT_UNSUPPORTED );  // "The requested format is is not supported"
   } else if ( hr == S_FALSE && spAudioFormatUsed != NULL ) {
      LOG_DEBUG_R( IDS_AUDIO_FORMAT_NOT_AVAILABLE );  // "The requested format is not available, but this format is:"
      audioPrintWaveFormat( spAudioFormatUsed );
      return FALSE;
   } else {
      RETURN_FATAL( IDS_AUDIO_FORMAT_INVALID );  // "Failed to validate the requested format"
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
      RETURN_FATAL( IDS_AUDIO_FAILED_TO_MATCH_FORMAT );  // "Failed to match with the audio format"
   }

   _ASSERTE( sAudioFormat != UNKNOWN_AUDIO_FORMAT );

   /// Initialize shared mode audio client
   //  Shared mode streams using event-driven buffering must set both periodicity and bufferDuration to 0.
   hr = spAudioClient->Initialize( sShareMode, AUDCLNT_STREAMFLAGS_EVENTCALLBACK
      | AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM, 0, 0, spMixFormat, NULL );
   if ( hr != S_OK ) {
      /// @todo Look at more error codes and print out higher-fidelity error messages
      RETURN_FATAL( IDS_AUDIO_FAILED_TO_INITIALIZE );  // "Failed to initialize the audio client"
   }

   hr = spAudioClient->GetBufferSize( &suBufferSize );
   CHECK_HR_R( IDS_AUDIO_FAILED_TO_GET_BUFFER_SIZE );  // "Failed to get buffer size"
   LOG_INFO_R( IDS_AUDIO_BUFFER_CAPACITY,  // "The maximum capacity of the buffer is %" PRIu32" frames or %i ms"
      suBufferSize,
      (int) ( 1.0 / spMixFormat->nSamplesPerSec * 1000 * suBufferSize ) );
   /// Right now, the buffer is ~22ms or about the perfect size to capture
   /// VoIP voice, which is 20ms.

   /// Get the device period
   hr = spAudioClient->GetDevicePeriod( &sDefaultDevicePeriod, &sMinimumDevicePeriod );
   CHECK_HR_R( IDS_AUDIO_FAILED_TO_GET_DEVICE_PERIODS );  // "Failed to get audio client device periods"

   LOG_INFO_R( IDS_AUDIO_DEFAULT_DEVICE_PERIOD, sDefaultDevicePeriod / 10000 );  // "Default device period=%lli ms"
   LOG_INFO_R( IDS_AUDIO_MINIMUM_DEVICE_PERIOD, sMinimumDevicePeriod / 10000 );  // "Minimum device period=%lli ms"


   /// Initialize the DTMF buffer
   br = pcmSetQueueSize( (size_t) spMixFormat->nSamplesPerSec / 1000 * SIZE_OF_QUEUE_IN_MS );
   CHECK_BR_R( IDS_AUDIO_FAILED_PCM_MALLOC );  // "Failed to allocate PCM queue"

   LOG_INFO_R( IDS_AUDIO_QUEUE_SIZE, gstQueueSize, SIZE_OF_QUEUE_IN_MS );  // "Queue size=%zu bytes or %d ms"

   br = goertzel_Start( spMixFormat->nSamplesPerSec );
   CHECK_BR_R( IDS_AUDIO_FAILED_TO_START_GOERTZEL );       // "Failed to start Goertzel DFT worker threads.  Exiting."

   hr = spAudioClient->SetEventHandle( ghAudioSamplesReadyEvent );
   CHECK_HR_R( IDS_AUDIO_FAILED_TO_SET_EVENT_CALLBACK );  // "Failed to set audio capture ready event"

   /// Get the Capture Client
   hr = spAudioClient->GetService( IID_PPV_ARGS( &spCaptureClient ) );
   CHECK_HR_R( IDS_AUDIO_FAILED_TO_GET_CAPTURE_CLIENT );  // "Failed to get capture client"

   /// Start the thread
   shCaptureThread = CreateThread( NULL, 0, audioCaptureThread, NULL, 0, NULL );
   if ( shCaptureThread == NULL ) {
      RETURN_FATAL( IDS_AUDIO_FAILED_TO_CREATE_CAPTURE_THREAD );  // "Failed to create the audio capture thread"
   }

   /// Start the audio processer
   hr = spAudioClient->Start();
   CHECK_HR_R( IDS_AUDIO_FAILED_TO_START_CAPTURE_STREAM );  // "Failed to start capturing the audio stream"

   /// Enable the `End Capture` menu item
   br = EnableMenuItem( ghMainMenu, IDM_AUDIO_ENDCAPTURE, MF_ENABLED );
   if ( br == -1 ) {
      RETURN_FATAL( IDS_AUDIO_FAILED_TO_SET_MENU_STATE );  // "Failed to set menu state.  Exiting."
   }

   LOG_INFO_R( IDS_AUDIO_START_SUCCESSFUL );  // "The audio capture device has started."

   return TRUE;
}


/// Stop the audio device and threads.  Unwind everything done in #audioStart
///
/// This function should not return until the audio thread **and** all of the
/// goertzel work threads have stopped.
///
/// In Win32, threads will set their signalled state when they terminate, so
/// let's take advantage of that.
///
/// @return `TRUE` if successful.  `FALSE` if there was a problem.
BOOL audioStop() {
   HRESULT hr;  // HRESULT result
   BOOL br;     // BOOL result

   _ASSERTE( ghMainWindow != NULL );
   _ASSERTE( ghMainMenu != NULL );

   /// Disable the `End Capture` menu item
   br = EnableMenuItem( ghMainMenu, IDM_AUDIO_ENDCAPTURE, MF_DISABLED );
   if ( br == -1 ) {
      RETURN_FATAL( IDS_AUDIO_FAILED_TO_SET_MENU_STATE );  // "Failed to set menu state.  Exiting."
   }

   /// Start by setting #gbIsRunning to `FALSE` -- just to be sure
   gbIsRunning = false;

   _ASSERTE( spAudioClient != NULL );
   _ASSERTE( ghAudioSamplesReadyEvent != NULL );
   _ASSERTE( shCaptureThread != NULL );

   hr = spAudioClient->Stop();
   CHECK_HR_R( IDS_AUDIO_STOP_FAILED );  // "Stopping the audio stream returned an unexpected value.  Investigate!!"

   /// Trigger the audio capture threads to loop...
   /// with #gbIsRunning `== FALSE` causing the loop to terminate
   br = SetEvent( ghAudioSamplesReadyEvent );
   CHECK_BR_R( IDS_AUDIO_FAILED_TO_SIGNAL_THREAD );  // "Failed to signal an audio capture thread.  Exiting."

   /// Wait for the thread to terminate
   DWORD dwWaitResult;  // Result from WaitForMultipleObjects
   dwWaitResult = WaitForSingleObject( shCaptureThread, INFINITE );
   if ( dwWaitResult != WAIT_OBJECT_0 ) {
      RETURN_FATAL( IDS_AUDIO_THREAD_END_FAILED );  // "Wait for the audio capture thread to end failed.  Exiting."
   }

   // At this point, the audio capture thread has ended
   br = CloseHandle( shCaptureThread );
   CHECK_BR_R( IDS_AUDIO_FAILED_CLOSING_THREAD );  // "Failed to close shCaptureThread"

   shCaptureThread = NULL;

   br = goertzel_Stop();
   WARN_BR_R( IDS_DTMF_DECODER_FAILED_TO_END_DFT_THREADS );  // "Failed to end the Goertzel DFT threads"

   SAFE_RELEASE( spCaptureClient );

   if ( spAudioClient != NULL ) {
      hr = spAudioClient->Reset();
      CHECK_HR_R( IDS_AUDIO_FAILED_TO_RELEASE_CLIENT );  // "Failed to release the audio client"
      spAudioClient = NULL;
   }

   pcmReleaseQueue();

   SAFE_RELEASE( spAudioClient );

   hr = PropVariantClear( &sDeviceFriendlyName );
   CHECK_HR_R( IDS_AUDIO_FAILED_TO_RELEASE_PROPERTY, L"Device Friendly Name" );  // "Failed to release property: %s"
   hr = PropVariantClear( &sDeviceDescription );
   CHECK_HR_R( IDS_AUDIO_FAILED_TO_RELEASE_PROPERTY, L"Device Description" );    // "Failed to release property: %s"
   hr = PropVariantClear( &sDeviceInterfaceFriendlyName );
   CHECK_HR_R( IDS_AUDIO_FAILED_TO_RELEASE_PROPERTY, L"Device Interface Friendly Name" );  // "Failed to release property: %s"

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

   /// Enable the `Start Capture` menu item
   br = EnableMenuItem( ghMainMenu, IDM_AUDIO_STARTCAPTURE, MF_ENABLED );
   if ( br == -1 ) {
      RETURN_FATAL( IDS_AUDIO_FAILED_TO_SET_MENU_STATE );  // "Failed to set menu state.  Exiting."
   }

   return TRUE;
}


/// Cleanup all things audio.  Unwind everything done in #audioInit
///
/// @return `TRUE` if successful.  `FALSE` if there was a problem.
BOOL audioCleanup() {
   BOOL    br;  // BOOL result

   /// @todo Do I need to unregister this event first?
   if ( ghAudioSamplesReadyEvent != NULL ) {
      br = CloseHandle( ghAudioSamplesReadyEvent );
      CHECK_BR_R( IDS_AUDIO_FAILED_CLOSING_EVENT );  // "Failed to close gAudioSamplesReadyEvent"
      ghAudioSamplesReadyEvent = NULL;
   }

   return TRUE;
}
