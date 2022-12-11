///////////////////////////////////////////////////////////////////////////////
//          University of Hawaii, College of Engineering
//          DTMF_Decoder - EE 469 - Fall 2022
//
//  A Windows Desktop C program that decodes DTMF tones
//
/// An 8-way multi-threaded Discrete Fast Forier Transform - specifically,
/// the Goertzel algorithm for 8-bit PCM data.
///
/// @see https://github.com/Harvie/Programs/blob/master/c/goertzel/goertzel.c
/// @see https://en.wikipedia.org/wiki/Goertzel_algorithm
/// @see https://en.wikipedia.org/wiki/Fast_Fourier_transform
///
/// ## Math APIs
/// | API                               | Link                                                                                             |
/// |-----------------------------------|--------------------------------------------------------------------------------------------------|
/// | `_USE_MATH_DEFINES`               | https://learn.microsoft.com/en-us/cpp/c-runtime-library/math-constants?view=msvc-170             |
/// | `sinf`                            | https://en.cppreference.com/w/c/numeric/math/sin                                                 |
/// | `cosf`                            | https://en.cppreference.com/w/c/numeric/math/cos                                                 |
/// | `sqrtf`                           | https://en.cppreference.com/w/c/numeric/math/sqrt                                                |
///
/// ## Threads & Synchronization API
/// | API                               | Link                                                                                             |
/// |-----------------------------------|--------------------------------------------------------------------------------------------------|
/// | `SetEvent`                        | https://learn.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-setevent                |
/// | `ResetEvent`                      | https://learn.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-resetevent              |
/// | `WaitForSingleObject`             | https://learn.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-waitforsingleobject     |
/// | `WaitForMultipleObjects`          | https://learn.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-waitformultipleobjects  |
/// | `CreateEventW`                    | https://learn.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-createeventw            |
///
/// ## Audio API
/// | API                               | Link                                                                                             |
/// |-----------------------------------|--------------------------------------------------------------------------------------------------|
/// | `AvSetMmThreadCharacteristics`    | https://learn.microsoft.com/en-us/windows/win32/api/avrt/nf-avrt-avsetmmthreadcharacteristicsa   |
/// | `AvRevertMmThreadCharacteristics` | https://learn.microsoft.com/en-us/windows/win32/api/avrt/nf-avrt-avrevertmmthreadcharacteristics |
///
/// ## Other API Calls
/// | API                               | Link                                                                                             |
/// |-----------------------------------|--------------------------------------------------------------------------------------------------|
/// | `CloseHandle`                     | https://learn.microsoft.com/en-us/windows/win32/api/handleapi/nf-handleapi-closehandle           |
/// | `#pragma message`                 | https://learn.microsoft.com/en-us/cpp/preprocessor/message?view=msvc-170                         |
/// | `HIWORD`                          | https://learn.microsoft.com/en-us/previous-versions/windows/desktop/legacy/ms632657(v=vs.85)     |
///
/// @file    goertzel.cpp
/// @author  Mark Nelson <marknels@hawaii.edu>
///////////////////////////////////////////////////////////////////////////////

#include "framework.h"    // Standard system include files
#include <avrt.h>         // For AvSetMmThreadCharacteristics()
#include <stdio.h>        // For sprintf_s()

/// For getting math defines in C++ (this is a .cpp file)
#define _USE_MATH_DEFINES
#include <math.h>         // For sinf() and cosf()

#include "DTMF_Decoder.h" // For APP_NAME
#include "mvcModel.h"     // For gPcmQueue and friends
#include "goertzel.h"     // For yo bad self


       HANDLE ghStartDFTevent;                          ///< All of the DFT threads wait to start on this handle.  Declared external to support inlining.
       HANDLE ghDoneDFTevent[ NUMBER_OF_DTMF_TONES ];   ///< The audio handler waits on each of the DFT threads before continuing.  Declared external to support inlining.
static HANDLE shWorkThreads[ NUMBER_OF_DTMF_TONES ];    ///< Handles to the worker threads

extern "C" float fScaleFactor = 0;  ///< Set in #goertzel_Init and used in #goertzel_Magnitude


#ifdef _WIN64
   #pragma message( "Compiling 64-bit program" )

   extern "C" {
      void goertzel_Magnitude_x64( const UINT8 index, dtmfTones_t* toneStruct );
   };
#else
   #pragma message( "Compiling 32-bit program" )
#endif


#ifndef _WIN64
/// Compute the Goertzel magnitude of 8-bit PCM data
///
/// This 1-pass loop over #gPcmQueue has been optimized for performance as it is
/// processing audio data in realtime.
///
/// The original version of this algorithm came from:
/// https://github.com/Harvie/Programs/blob/master/c/goertzel/goertzel.c
///
/// Inlined for performance.
///
/// @param index       The index into the DTMF tones array
/// @param toneStruct  A pointer to #gDtmfTones (so it doesn't have to
///                    re-compute the index each time
__forceinline static void goertzel_Magnitude(
   _In_     const UINT8        index,
   _Inout_        dtmfTones_t* toneStruct ) {

   _ASSERTE( gstQueueHead < gstQueueSize );
   _ASSERTE( gstQueueSize > 0 );

   float real, imag;

   float q1 = 0;
   float q2 = 0;

   size_t queueRead = gstQueueHead;  // Thread safe way to point to the next available byte for reading

   for ( size_t i = 0; i < gstQueueSize; i++ ) {
      float q0 = toneStruct->coeff * q1 - q2 + ( (float) gPcmQueue[ queueRead++ ] );
      q2 = q1;
      q1 = q0;
      if ( queueRead >= gstQueueSize ) { // Wrap around at the end of the queue
         queueRead = 0;
      }
   }

   // Calculate the real and imaginary results scaling appropriately
   real = ( q1 * toneStruct->cosine - q2 );
   imag = ( q1 * toneStruct->sine );

   toneStruct->goertzelMagnitude = sqrtf( real * real + imag * imag ) / fScaleFactor;
}
#endif


/// Runs each of the 8 DFT worker threads
///
/// @see https://learn.microsoft.com/en-us/previous-versions/windows/desktop/legacy/ms686736(v=vs.85)
///
/// @param pContext Holds the index of which tone this thread is responsibile
///                 for values are `0` through `7`.  This is critical for
///                 thread safety.
///
/// @return `0` if successful.  Non-`0` if there was a problem.
DWORD WINAPI goertzelWorkThread( _In_ LPVOID pContext ) {
   _ASSERTE( pContext != NULL );

   int    iIndex = *(int*) pContext;  // Comes to us as an int from #dtmfTones_t
   size_t index  = iIndex;            // But we use it as an index into an array, so convert to `size_t`

   _ASSERTE( iIndex < NUMBER_OF_DTMF_TONES );
   _ASSERTE( ghStartDFTevent != NULL );
   _ASSERTE( ghDoneDFTevent[ index ]  != NULL );

   LOG_TRACE_R( IDS_GOERTZEL_START, index );  // "Goertzel DFT thread: %zu   Starting."

   HANDLE mmcssHandle = NULL;  // Local to the thread for safety

   /// Set the multimedia class scheduler service, which will set the CPU
   /// priority for this thread
   ///
   /// Uses the exported #gdwMmcssTaskIndex originally set in audio.cpp
   ///
   /// @see https://learn.microsoft.com/en-us/windows/win32/api/avrt/nf-avrt-avsetmmthreadcharacteristicsa
   mmcssHandle = AvSetMmThreadCharacteristicsW( L"Capture", &gdwMmcssTaskIndex );
   if ( mmcssHandle == NULL ) {
      LOG_WARN_R( IDS_GOERTZEL_FAILED_TO_SET_MMCSS, iIndex );  // "Goertzel DFT thread: %zu   Failed to set MMCSS on Goertzel work thread.  Continuing."
   }
   // LOG_INFO_R( IDS_GOERTZEL_SET_MMCSS, iIndex );  // "Goertzel DFT thread: %zu   Set MMCSS on Goertzel work thread."

   while ( gbIsRunning ) {
      DWORD dwWaitResult;

      /// Wait for the #ghStartDFTevent to be signalled (shared by all DFT threads)
      dwWaitResult = WaitForSingleObject( ghStartDFTevent, INFINITE);
      if ( dwWaitResult == WAIT_OBJECT_0 ) {
         if ( gbIsRunning ) {
            #ifdef _WIN64
               goertzel_Magnitude_x64( (UINT8) index, &gDtmfTones[index] );
            #else
              goertzel_Magnitude( iIndex, &gDtmfTones[index] );
            #endif

            if ( gDtmfTones[index].goertzelMagnitude >= GOERTZEL_MAGNITUDE_THRESHOLD ) {
               mvcModelToggleToneDetectedStatus( index, true );
            } else {
               mvcModelToggleToneDetectedStatus( index, false );
            }
         }
      } else if ( dwWaitResult == WAIT_FAILED ) {
         QUEUE_FATAL( IDS_GOERTZEL_WAITFORSINGLEOBJECT_FAILED, iIndex );  // "WaitForSingleObject in Goertzel thread %zu failed.  Exiting.  Investigate!"
      } else {
         QUEUE_FATAL( IDS_GOERTZEL_WAITFORSINGLEOBJECT_FAILED_UNKNOWN, iIndex );  // "WaitForSingleObject in Goertzel thread %zu ended for an unknown reason.  Exiting.  Investigate!"
      }

      SetEvent( ghDoneDFTevent[ index ] );
   }

   /// When the thread is done, put the CPU thread priority back
   if ( mmcssHandle != NULL ) {
      if( !AvRevertMmThreadCharacteristics( mmcssHandle ) ) {
         LOG_WARN_Q( IDS_GOERTZEL_FAILED_TO_REVERT_MMCSS, iIndex );  // "Goertzel DFT thread: %zu   Failed to revert MMCSS on Goertzel work thread.  Continuing."
      }
      mmcssHandle = NULL;
   }

   LOG_TRACE_R( IDS_GOERTZEL_DONE, index );  // "Goertzel DFT thread: %zu   Done"

   ExitThread( 0 );
}


/// Define PI as a floating point number
#define M_PIF 3.141592653589793238462643383279502884e+00F


/// Initialize values needed by the goertzel DFT and start the worker threads
///
/// @param  iSampleRate  Samples per second
/// @return `TRUE` if successful.  `FALSE` if there was a problem.
BOOL goertzel_Init( _In_ const int iSampleRate ) {
   _ASSERTE( iSampleRate > 0 );
   _ASSERTE( gstQueueSize > 0 );

   float floatSamplingRate = (float) iSampleRate;

   float floatNumSamples = (float) gstQueueSize;

   fScaleFactor = gstQueueSize / 2.0f;

   /// Set sine, cosine and coeff for each DTMF tone in #gDtmfTones.
   for ( int i = 0 ; i < NUMBER_OF_DTMF_TONES ; i++ ) {
      int   k      = (int) ( 0.5f + ( ( floatNumSamples * gDtmfTones[ i ].frequency ) / (float) floatSamplingRate ) );
      float omega  = ( 2.0f * M_PIF * k ) / floatNumSamples;

      gDtmfTones[ i ].sine   = sinf( omega );
      gDtmfTones[ i ].cosine = cosf( omega );
      gDtmfTones[ i ].coeff  = 2.0f * gDtmfTones[ i ].cosine;
   }

   /// Create events for synchronizing the threads
   _ASSERTE( ghStartDFTevent == NULL );
   ghStartDFTevent = CreateEventW(
      NULL,                            // Default security attributes
      TRUE,                            // Manual-reset option
      FALSE,                           // Initial state
      NULL );                          // Object name
   if ( ghStartDFTevent == NULL ) {
      RETURN_FATAL( IDS_GOERTZEL_FAILED_TO_CREATE_STARTDFT_HANDLE );  // "Failed to create a ghStartDFTevent event handle.  Exiting."
   }

   for ( int i = 0 ; i < NUMBER_OF_DTMF_TONES ; i++ ) {
      _ASSERTE( ghDoneDFTevent[ i ] == NULL );
      _ASSERTE( shWorkThreads[ i ] == NULL );

      ghDoneDFTevent[ i ] = CreateEventW(
         NULL,                         // Default security attributes
         FALSE,                        // Manual-reset option
         FALSE,                        // Initial state
         NULL );                       // Object name
      if ( ghDoneDFTevent[ i ] == NULL ) {
         RETURN_FATAL( IDS_GOERTZEL_FAILED_TO_CREATE_DONEDFT_HANDLES );  // "Failed to create a ghDoneDFTevent event handle.  Exiting."
      }

      /// Start the threads
      shWorkThreads[i] = CreateThread(NULL, 0, goertzelWorkThread, &gDtmfTones[i].index, 0, NULL);
      if ( shWorkThreads[ i ] == NULL ) {
         RETURN_FATAL( IDS_GOERTZEL_FAILED_TO_CREATE_WORK_THREAD );  // "Failed to create a Goertzel work thread.  Exiting."
      }
   }

   return TRUE;
}


/// Signal all of the threads so they can end on their own terms
///
/// This function should not return until all of the goertzel work threads have
/// stopped.  In Win32, threads will set their signalled state when they
/// terminate, so let's take advantage of that.
///
/// @see https://learn.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-setevent
/// @see https://learn.microsoft.com/en-us/windows/win32/sync/wait-functions
///
/// @return `TRUE` if successful.  `FALSE` if there was a problem.
BOOL goertzel_Stop() {
   /// Start by setting #gbIsRunning to `FALSE` -- just to be sure
   gbIsRunning = false;

   BOOL br;  // BOOL result

   _ASSERTE( ghStartDFTevent != NULL );
   for ( int i = 0 ; i < NUMBER_OF_DTMF_TONES ; i++ ) {
      _ASSERTE( shWorkThreads[ i ] != NULL );
   }
   /// Trigger all of the threads to run... which will spin their loops
   /// with #gbIsRunning `== FALSE` causing all of the threads to terminate
   br = SetEvent( ghStartDFTevent );
   CHECK_BR_R( IDS_GOERTZEL_FAILED_TO_SIGNAL_START_DFT );  // "Failed to signal a ghStartDFTevent.  Exiting."

   /// Wait for the threads to terminate
   DWORD   dwWaitResult;  // Result from WaitForMultipleObjects
   dwWaitResult = WaitForMultipleObjects( NUMBER_OF_DTMF_TONES, shWorkThreads, TRUE, INFINITE );
   if ( !( dwWaitResult >= WAIT_OBJECT_0 && dwWaitResult <= ( WAIT_OBJECT_0 + NUMBER_OF_DTMF_TONES - 1 ) ) ) {
      RETURN_FATAL( IDS_GOERTZEL_THREAD_END_FAILED );  // "Wait for all Goertzel threads to end failed.  Exiting."
   }

   /// Cleanup the thread handles
   for ( int i = 0 ; i < NUMBER_OF_DTMF_TONES ; i++ ) {
      shWorkThreads[ i ] = NULL;
   }

   LOG_TRACE_R( IDS_GOERTZEL_ENDED_NORMALLY );  // "All Goertzel threads ended normally"

   return TRUE;
}


/// Cleanup goertzel event handles
///
/// @return `TRUE` if successful.  `FALSE` if there was a problem.
BOOL goertzel_Cleanup() {
   BOOL br;  // BOOL result

   if ( ghStartDFTevent != NULL ) {
      br = CloseHandle( ghStartDFTevent );
      CHECK_BR_R( IDS_GOERTZEL_FAILED_TO_CLOSE_STARTDFT_HANDLE );  //  "Failed to close ghStartDFTevent handle"
      ghStartDFTevent = NULL;
   }

   for ( int i = 0 ; i < NUMBER_OF_DTMF_TONES ; i++ ) {
      if ( ghDoneDFTevent[ i ] != NULL ) {
         br = CloseHandle( ghDoneDFTevent[ i ] );
         CHECK_BR_R( IDS_GOERTZEL_FAILED_TO_CLOSE_DONEDFT_HANDLES );  // "Failed to close ghDoneDFTevent handle"
         ghDoneDFTevent[ i ] = NULL;
      }
   }

   return TRUE;
}
