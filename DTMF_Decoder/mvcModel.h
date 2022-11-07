///////////////////////////////////////////////////////////////////////////////
//          University of Hawaii, College of Engineering
//          DTMF_Decoder - EE 469 - Fall 2022
//
//  A Windows Desktop C program that decodes DTMF tones
//
/// The model holds the state between the various modules
///
/// @todo Consider having the display have an indicator light that shows the
///       threads are running
///
/// @file    mvcModel.h
/// @version 1.0
///
/// @author  Mark Nelson <marknels@hawaii.edu>
/// @date    10_Oct_2022
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <Windows.h>      // For WCHAR, BYTE, etc.
#include "mvcView.h"      // for mvcInvalidateRow and mvcInvalidateColumn


/// The number of DTMF tones DTMF Decoder processes
#define NUMBER_OF_DTMF_TONES (8)


/// Initialize the model
extern BOOL mvcModelInit();


/// Hold display information (#detected & #label) as well as
/// pre-computed information for the Goertzel Magnitude calculation for each
/// individual DTMF tone
///
/// #sine, #cosine, #coeff are computed and set by #goertzel_init
///
/// #goertzelMagnitude is set in #goertzel_magnitude
///
/// #detected is set in #goertzelWorkThread
typedef struct {
   int   index;              ///< The index of the tone in the gDtmfTones array
   float frequency;          ///< The DTMF tone's frequency
   bool  detected;           ///< `true` if a tone is found, `false` if it's not
   WCHAR label[ 16 ];        ///< A Wide-char label for the tone
   float goertzelMagnitude;  ///< The latest magnitude
   float sine;               ///< Pre-computed sin offset for the frequency and sample rate
   float cosine;             ///< Pre-computed cosine offset for the frequency and smaple rate
   float coeff;              ///< Pre-computed Goertzel coefficient
} dtmfTones_t;


/// An array holding display information (#dtmfTones_t.detected &
/// #dtmfTones_t.label) as well as pre-computed information
/// for the Goertzel Magnitude calculation for each individual DTMF tone
extern dtmfTones_t gDtmfTones[ NUMBER_OF_DTMF_TONES ];


/// Determine if the state of a DTMF tone detection has changed.  If it has,
/// then we need to invalidate that region of the display.
inline void mvcModelToggleToneDetectedStatus(
   _In_ const size_t toneIndex,
   _In_ const bool   detectedStatus ) {

   _ASSERTE( toneIndex < NUMBER_OF_DTMF_TONES );

   if ( gDtmfTones[ toneIndex ].detected != detectedStatus ) {
      gDtmfTones[ toneIndex ].detected = detectedStatus;

      BOOL br;  // BOOL result

      if ( ( toneIndex & 0b1100 ) == 0 ) {  // If toneIndex is between 0 and 3
         br = mvcInvalidateRow( toneIndex );
      } else {
         br = mvcInvalidateColumn( toneIndex - 4 );  // toneIndex is between 4 and 7,
      }                                              // which turns into columns 0 through 3

      WARN_BR( "Failed to invalidate region of screen" );
   }
}


/// When `true`, #audioCaptureThread and #goertzelWorkThread event blocking
/// loops will continue to run.
///
/// Set to `false` when it's time to shutdown the program.  Then, these threads
/// will see that #gbIsRunning is `false`, drop out of their `while()` loops and
/// the threads will terminate naturally, cleaning up their resources.
///
/// @internal This is a very important variable as it's what keeps the loops
///           running.
///
extern bool gbIsRunning;


/// Pointer to the main window handle
extern HWND ghMainWindow;


/// The size of the queue in milliseconds.  This determines the number of
/// samples the Goertzel DFT #goertzel_magnitude uses to analyze the signal.
///
/// Generally, the larger the queue, the slower (but more accurate) the
/// detection is.
///
/// The standard is 65ms
/// @see https://www.etsi.org/deliver/etsi_es/201200_201299/20123502/01.01.01_60/es_20123502v010101p.pdf
#define SIZE_OF_QUEUE_IN_MS (65)


/// Pointer to the #gPcmQueue.  The queue is allocated by #pcmSetQueueSize.
/// Released by #pcmReleaseQueue.  It is populated in #processAudioFrame
/// by #pcmEnqueue.  #goertzel_magnitude needs direct access to this to analyze
/// the audio stream.
///
/// This is thread safe because all of the threads read from the same,
/// unchanging queue.
///
/// @internal #gPcmQueue would normally be protected by an API, but because
///           of the realtime nature of this application, performance is
///           critical.  Therefore, we are allowing other modules direct access
///           to this data structure.
///
/// @internal This is declared as `extern "C"` as it is accessed by an
///           Assembluy Language module.  `extern "C"` disables C++'s name
///           mangling and allows access from Assembly.
extern "C" BYTE*  gPcmQueue;


/// A relative offset within #gPcmQueue of the next available byte for
/// writing.  `0` is the first available byte in the queue.  This value should
/// never be `>=` #gstQueueSize.
///
/// This is thread safe because all of the threads read from the same,
/// unchanging queue.
///
/// @internal This is declared as `extern "C"` as it is accessed by an
///           Assembluy Language module.  `extern "C"` disables C++'s name
///           mangling and allows access from Assembly.
extern "C" size_t gstQueueHead;


/// Represents the maximum size of #gPcmQueue.  This is set in #pcmSetQueueSize
/// called by #audioInit after we know the sampling rate (`gpMixFormat->nSamplesPerSec`).
///
/// Size in bytes of DTMF DFT #gPcmQueue `= gpMixFormat->nSamplesPerSec / 1000 * SIZE_OF_QUEUE_IN_MS`
///
/// The queue is sized to hold 8-bit PCM data (one byte per sample)
///
/// This is thread safe because all of the threads read from the same,
/// unchanging queue.
///
/// @internal This is declared as `extern "C"` as it is accessed by an
///           Assembluy Language module.  `extern "C"` disables C++'s name
///           mangling and allows access from Assembly.
extern "C" size_t gstQueueSize;


/// Set the size of #gPcmQueue, allocate and zero the space for it.
///
/// Uses `_malloc_dbg`
/// @see https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/malloc-dbg?view=msvc-170
extern BOOL pcmSetQueueSize( _In_ const size_t size );


/// Enqueue a byte of PCM data to `gPcmQueue`
///
/// Delcared `inline` for performance reasons
inline void pcmEnqueue( _In_ const BYTE data ) {
   _ASSERTE( gPcmQueue != NULL );
   _ASSERTE( gstQueueHead < gstQueueSize );

   gPcmQueue[ gstQueueHead++ ] = data ;

   if ( gstQueueHead >= gstQueueSize ) {  // More efficient than `gstQueueHead %= gstQueueSize`
      gstQueueHead = 0;
   }

   _ASSERTE( _CrtCheckMemory() );
}


/// Release the memory allocated to #gPcmQueue
extern void pcmReleaseQueue();


/// Common handle for audio task prioritization
///
/// @see https://learn.microsoft.com/en-us/windows/win32/api/avrt/nf-avrt-avsetmmthreadcharacteristicsa
extern DWORD gdwMmcssTaskIndex;


/// This event is signaled when the audio driver has some data to send.
///
/// @see https://learn.microsoft.com/en-us/windows/win32/api/audioclient/nf-audioclient-iaudioclient-seteventhandle
extern HANDLE ghAudioSamplesReadyEvent;
