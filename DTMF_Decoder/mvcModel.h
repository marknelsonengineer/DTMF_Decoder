///////////////////////////////////////////////////////////////////////////////
//          University of Hawaii, College of Engineering
//          DTMF_Decoder - EE 469 - Fall 2022
//
//  A Windows Desktop C program that decodes DTMF tones
//
/// The model holds the state between the various modules
///
/// @file mvcModel.h
/// @version 1.0
///
/// @todo Consider having the display have an indicator light that shows the
///       threads are running
///
///
/// @author Mark Nelson <marknels@hawaii.edu>
/// @date   10_Oct_2022
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <Windows.h>      // For WCHAR, BYTE, etc.


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
   int   index;              ///< The index of the tone in the dtmfTones array
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
extern dtmfTones_t dtmfTones[ NUMBER_OF_DTMF_TONES ];


/// Temporarily set to `true` when the Goertzel DFT detects that any DTMF tone
/// has changed.
///
/// `false` if there are no recent changes (so there's no need to repaint the
/// screen).
extern bool hasDtmfTonesChanged;


/// Determine if the state of a DTMF tone detection has changed.  If it has,
/// then we need to repaint the display.  See #hasDtmfTonesChanged
inline void mvcModelToggleToneDetectedStatus( size_t toneIndex, bool detectedStatus ) {
   _ASSERTE( toneIndex < NUMBER_OF_DTMF_TONES );

   if ( dtmfTones[ toneIndex ].detected != detectedStatus ) {
      dtmfTones[ toneIndex ].detected = detectedStatus;
      hasDtmfTonesChanged = true;
   }
}


/// When `true`, #audioCaptureThread and #goertzelWorkThread event blocking
/// loops will continue to run.
///
/// Set to `false` when it's time to shutdown the program.  Then, these threads
/// will see that #isRunning is `false`, drop out of their `while()` loops and
/// the threads will terminate naturally, cleaning up their resources.
///
/// @internal This is a very important variable as it's what keeps the loops
///           running.
///
extern bool isRunning;


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


/// Pointer to the #pcmQueue.  The queue is allocated by #pcmSetQueueSize.
/// Released by #pcmReleaseQueue.  It is populated in #processAudioFrame
/// by #pcmEnqueue.  #goertzel_magnitude needs direct access to this to analyze
/// the audio stream.
///
/// This is thread safe because all of the threads read from the same,
/// unchanging queue.
///
/// @internal #pcmQueue would normally be protected by an API, but because
///           of the realtime nature of this application, performance is
///           critical.  Therefore, we are allowing other modules direct access
///           to this data structure.
///
/// @internal This is declared as `extern "C"` as it is accessed by an
///           Assembluy Language module.  `extern "C"` disables C++'s name
///           mangling and allows access from Assembly.
extern "C" BYTE*  pcmQueue;


/// Points to a relative offset within #pcmQueue of the next available byte for
/// writing.  `0` is the first available byte in the queue.  This value should
/// never be `>=` #queueSize.
///
/// This is thread safe because all of the threads read from the same,
/// unchanging queue.
///
/// @internal This is declared as `extern "C"` as it is accessed by an
///           Assembluy Language module.  `extern "C"` disables C++'s name
///           mangling and allows access from Assembly.
extern "C" size_t queueHead;


/// Represents the maximum size of #pcmQueue.  This is set in #pcmSetQueueSize
/// called by #audioInit after we know the sampling rate (`gpMixFormat->nSamplesPerSec`).
///
/// Size in bytes of DTMF DFT #pcmQueue `= gpMixFormat->nSamplesPerSec / 1000 * SIZE_OF_QUEUE_IN_MS`
///
/// The queue is sized to hold 8-bit PCM data (one byte per sample)
///
/// This is thread safe because all of the threads read from the same,
/// unchanging queue.
///
/// @internal This is declared as `extern "C"` as it is accessed by an
///           Assembluy Language module.  `extern "C"` disables C++'s name
///           mangling and allows access from Assembly.
extern "C" size_t queueSize;


/// Set the size of #pcmQueue, allocate and zero the space for it.
///
/// Uses `_malloc_dbg`
/// @see https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/malloc-dbg?view=msvc-170
extern BOOL pcmSetQueueSize( _In_ size_t size );


/// Enqueue a byte of PCM data to `pcmQueue`
///
/// Delcared `inline` for performance reasons
inline void pcmEnqueue( _In_ BYTE data ) {
   _ASSERTE( pcmQueue != NULL );
   _ASSERTE( queueHead < queueSize );

   pcmQueue[ queueHead++ ] = data ;

   if ( queueHead >= queueSize ) {     // More efficient than `queueHead %= queueSize`
      queueHead = 0;
   }

   // _ASSERTE( _CrtCheckMemory() );   // Asserts removed for performance
}


/// Release the memory allocated to #pcmQueue
extern void pcmReleaseQueue();


/// Common handle for audio task prioritization
///
/// @see https://learn.microsoft.com/en-us/windows/win32/api/avrt/nf-avrt-avsetmmthreadcharacteristicsa
extern DWORD mmcssTaskIndex;


/// This event is signaled when the audio driver has some data to send.
///
/// @see https://learn.microsoft.com/en-us/windows/win32/api/audioclient/nf-audioclient-iaudioclient-seteventhandle
extern HANDLE gAudioSamplesReadyEvent;
