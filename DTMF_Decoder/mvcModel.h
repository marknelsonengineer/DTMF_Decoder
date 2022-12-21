///////////////////////////////////////////////////////////////////////////////
//          University of Hawaii, College of Engineering
//          DTMF_Decoder - EE 469 - Fall 2022
//
//  A Windows Desktop C program that decodes DTMF tones
//
/// The model holds the state between the various modules
///
/// ### APIs Used
/// << Print Module API Documentation >>
///
/// @file    mvcModel.h
/// @author  Mark Nelson <marknels@hawaii.edu>
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <Windows.h>      // For WCHAR, BYTE, etc.
#include "mvcView.h"      // For mvcInvalidateRow and mvcInvalidateColumn


/// The number of tones DTMF Decoder processes
#define NUMBER_OF_DTMF_TONES (8)


/// The size of the queue in milliseconds.  This determines the number of
/// samples #goertzel_Magnitude uses to analyze the signal.
///
/// Generally, the larger the queue, the slower (but more accurate) the
/// detection is.
///
/// The standard is 65ms
/// @see https://www.etsi.org/deliver/etsi_es/201200_201299/20123502/01.01.01_60/es_20123502v010101p.pdf
#define SIZE_OF_QUEUE_IN_MS (65)


extern BOOL mvcModelInit();

extern BOOL mvcModelRelease();


/// Hold display information (#detected & #label) as well as
/// pre-computed information for the Goertzel DFT magnitude calculation for
/// of the individual DTMF tones
///
/// #sine, #cosine, #coeff are computed in #goertzel_Start
///
/// #goertzelMagnitude is set in #goertzel_Magnitude
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
/// for the Goertzel DFT magnitude calculation for each individual DTMF tone
extern dtmfTones_t gDtmfTones[ NUMBER_OF_DTMF_TONES ];


/// When `true`, #audioCaptureThread and #goertzelWorkThread loops run.
///
/// Set to `false` when it's time to shutdown.  The `while()` loops will see
/// that #gbIsRunning is `false` and exit.  Then, the threads will terminate
/// naturally, cleaning up their resources.
///
/// @internal This is a very important variable as it's what keeps the loops
///           running.
///
extern bool gbIsRunning;


/// Handle to the main window.  This is really a pointer.
extern HWND ghMainWindow;


/// Handle to the main window's menu.  This is really a pointer.
extern HMENU ghMainMenu;


/// The application's return value.  Defaults to 0 (`EXIT_SUCCESS`).  Any error
/// handler can set this which will be passed out of the program when it
/// terminates.
extern int giApplicationReturnValue;


/// Pointer to #gPcmQueue.  The queue is allocated by #pcmSetQueueSize.
/// Released by #pcmReleaseQueue.  It is populated in #processAudioFrame
/// by #pcmEnqueue.  #goertzel_Magnitude needs direct access to this to analyze
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


/// The maximum size of #gPcmQueue.  This is set in #pcmSetQueueSize
/// called by #audioInit after we know the sampling rate (`gpMixFormat->nSamplesPerSec`).
///
/// Size in bytes of DTMF DFT #gPcmQueue `= gpMixFormat->nSamplesPerSec / 1000 * SIZE_OF_QUEUE_IN_MS`.
///
/// The queue is sized to hold 8-bit PCM data (one byte per sample).
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
extern BOOL pcmSetQueueSize( _In_ const size_t size );


/// Enqueue a byte of PCM data to `gPcmQueue`
///
/// Inlined for performance.
///
/// Delcared `inline` for performance reasons
__forceinline void pcmEnqueue( _In_ const BYTE data ) {
   _ASSERTE( gPcmQueue != NULL );
   _ASSERTE( gstQueueHead < gstQueueSize );

   gPcmQueue[ gstQueueHead++ ] = data ;

   if ( gstQueueHead >= gstQueueSize ) {  // More efficient than `gstQueueHead %= gstQueueSize`
      gstQueueHead = 0;
   }

   _ASSERTE( _CrtCheckMemory() );
}


/// Release memory allocated to #gPcmQueue
extern void pcmReleaseQueue();


/// Common handle for audio task prioritization
///
/// @see https://learn.microsoft.com/en-us/windows/win32/api/avrt/nf-avrt-avsetmmthreadcharacteristicsw
extern DWORD gdwMmcssTaskIndex;


/// This event is signaled when the audio driver has some data to send.
///
/// @see https://learn.microsoft.com/en-us/windows/win32/api/audioclient/nf-audioclient-iaudioclient-seteventhandle
extern HANDLE ghAudioSamplesReadyEvent;


/// Invalidate the column (not the whole screen)
///
/// Inlined for performance.
///
/// @param column Index of the column... 0 through 3.
///
/// @return `TRUE` if successful.  `FALSE` if there was a problem.
__forceinline BOOL mvcInvalidateColumn( _In_ const size_t column ) {
   _ASSERTE( column <= 3 );
   _ASSERTE( ghMainWindow != NULL );

   /// #### Function

   BOOL br;                    // BOOL result
   RECT rectToRedraw = { 0 };  // The rectangle to redraw

   rectToRedraw.top = 0;
   rectToRedraw.bottom = giWindowHeight;

   switch ( column ) {
      case 0:
         rectToRedraw.left = COL0 - 16;
         rectToRedraw.right = COL0 + 71;
         break;
      case 1:
         rectToRedraw.left = COL1 - 16;
         rectToRedraw.right = COL1 + 71;
         break;
      case 2:
         rectToRedraw.left = COL2 - 16;
         rectToRedraw.right = COL2 + 71;
         break;
      case 3:
         rectToRedraw.left = COL3 - 16;
         rectToRedraw.right = COL3 + 71;
         break;
   }

   /// - Based on the column, invalidate the appropriate display region with InvalidateRect
   br = InvalidateRect( ghMainWindow, &rectToRedraw, FALSE );
   if ( !br ) {
      QUEUE_FATAL( IDS_MODEL_FAILED_TO_INVALIDATE_COLUMN, column );  // "Failed to invalidate column %zu"
      return FALSE;
   }

   return TRUE;
}


/// Invalidate just the row (not the whole screen)
///
/// Inlined for performance.
///
/// @param row Index of the row... 0 through 3.
///
/// @return `TRUE` if successful.  `FALSE` if there was a problem.
__forceinline BOOL mvcInvalidateRow( _In_ const size_t row ) {
   _ASSERTE( row <= 3 );
   _ASSERTE( ghMainWindow != NULL );

   /// #### Function

   BOOL br;                    // BOOL result
   RECT rectToRedraw = { 0 };  // The rectangle to redraw

   rectToRedraw.left = 0;
   rectToRedraw.right = giWindowWidth;

   switch ( row ) {
      case 0:
         rectToRedraw.top = ROW0;
         rectToRedraw.bottom = ROW0 + BOX_HEIGHT;
         break;
      case 1:
         rectToRedraw.top = ROW1;
         rectToRedraw.bottom = ROW1 + BOX_HEIGHT;
         break;
      case 2:
         rectToRedraw.top = ROW2;
         rectToRedraw.bottom = ROW2 + BOX_HEIGHT;
         break;
      case 3:
         rectToRedraw.top = ROW3;
         rectToRedraw.bottom = ROW3 + BOX_HEIGHT;
         break;
   }

   /// - Based on the row, invalidate the appropriate display region with InvalidateRect
   br = InvalidateRect( ghMainWindow, &rectToRedraw, FALSE );
   if ( !br ) {
      QUEUE_FATAL( IDS_MODEL_FAILED_TO_INVALIDATE_ROW, row );  // "Failed to invalidate row %zu"
      return FALSE;
   }

   return TRUE;
}


/// Determine if the state of a DTMF tone has changed.  If it has, invalidate
/// that region of the display.
///
/// @param toneIndex      Index of the DTMF tone.
/// @param detectedStatus Based on the Goertzel DFT, is the tone detected
///                       (`true`) or not (`false`).
///
/// Inlined for performance.
__forceinline void mvcModelToggleToneDetectedStatus(
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

      if ( !br ) {
         QUEUE_FATAL( IDS_MODEL_FAILED_TO_INVALIDATE_REGION, toneIndex );  // "Goertzel DFT thread %zu   Failed to invalidate region of screen"
      }
   }
}
