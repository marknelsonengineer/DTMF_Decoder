///////////////////////////////////////////////////////////////////////////////
//          University of Hawaii, College of Engineering
//          DTMF_Decoder - EE 469 - Fall 2022
//
//  A Windows Desktop C program that decodes DTMF tones
//
/// An 8-way multi-threaded Discrete Fast Forier Transform.  This implements
/// a Goertzel algorithm for analyzing the energy in the 8 DTMF frquencies
/// in an 8-bit PCM audio stream.
///
/// @see https://github.com/Harvie/Programs/blob/master/c/goertzel/goertzel.c
/// @see https://en.wikipedia.org/wiki/Goertzel_algorithm
/// @see https://en.wikipedia.org/wiki/Fast_Fourier_transform
///
/// ### APIs Used
/// << Print Module API Documentation >>
///
/// @file    goertzel.cpp
/// @author  Mark Nelson <marknels@hawaii.edu>
///////////////////////////////////////////////////////////////////////////////

#include "framework.h"    // Standard system include files
#include <avrt.h>         // For AvSetMmThreadCharacteristics()
#include <stdio.h>        // For sprintf_s()

/// _USE_MATH_DEFINES is for getting math defines in C++ (this is a .cpp file)
#define _USE_MATH_DEFINES
#include <math.h>         // For sinf() and cosf()

#include "DTMF_Decoder.h" // For APP_NAME
#include "mvcModel.h"     // For gPcmQueue and friends
#include "goertzel.h"     // For yo bad self


/// A floating point version of PI
#define M_PIF 3.141592653589793238462643383279502884e+00F

/// All of the DFT work threads wait to start on this handle.
/// Declared external to support inlining.
HANDLE ghStartDFTevent = NULL;

/// The #audioCaptureThread waits on each of the DFT work threads before continuing.
/// Declared external to support inlining.
HANDLE ghDoneDFTevents[ NUMBER_OF_DTMF_TONES ] = { NULL };

/// Array of handles to the 8 work threads
static HANDLE shWorkThreads[ NUMBER_OF_DTMF_TONES ] = { NULL };

extern "C" float sfScaleFactor = 0;  ///< Set in #goertzel_Start and used in #goertzel_Magnitude


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


/// Runs each of the 8 DFT work threads
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
   _ASSERTE( ghDoneDFTevents[ index ]  != NULL );

   /// #### Function

   LOG_TRACE_R( IDS_GOERTZEL_START, index );  // "Goertzel DFT thread: %zu   Starting."

   HANDLE mmcssHandle = NULL;  // Local to the thread for safety

   /// - Set the CPU priority for this thread with AvSetMmThreadCharacteristicsW
   ///     - Uses the exported #gdwMmcssTaskIndex originally set in #audioStart
   mmcssHandle = AvSetMmThreadCharacteristicsW( L"Capture", &gdwMmcssTaskIndex );
   if ( mmcssHandle == NULL ) {
      LOG_WARN_R( IDS_GOERTZEL_FAILED_TO_SET_MMCSS, iIndex );  // "Goertzel DFT thread: %zu   Failed to set MMCSS on Goertzel work thread.  Continuing."
   }
   // LOG_INFO_R( IDS_GOERTZEL_SET_MMCSS, iIndex );  // "Goertzel DFT thread: %zu   Set MMCSS on Goertzel work thread."

   /// - Start a loop while #gbIsRunning is `true`
   while ( gbIsRunning ) {
      DWORD dwWaitResult;

      /// - Wait for #ghStartDFTevent to be signalled (shared by all DFT threads)
      ///   with WaitForSingleObject
      dwWaitResult = WaitForSingleObject( ghStartDFTevent, INFINITE);
      if ( dwWaitResult == WAIT_OBJECT_0 ) {
         ///     - Compute the energy in given DTMF frequency using #goertzel_Magnitude
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

      /// - Tell the audio processing thread that this thread's DFT processing is
      ///   done with SetEvent
      SetEvent( ghDoneDFTevents[ index ] );
   }

   /// - When the loop is done, restore the thread's priority with
   ///   AvRevertMmThreadCharacteristics
   if ( mmcssHandle != NULL ) {
      if( !AvRevertMmThreadCharacteristics( mmcssHandle ) ) {
         LOG_WARN_Q( IDS_GOERTZEL_FAILED_TO_REVERT_MMCSS, iIndex );  // "Goertzel DFT thread: %zu   Failed to revert MMCSS on Goertzel work thread.  Continuing."
      }
      mmcssHandle = NULL;
   }

   LOG_TRACE_R( IDS_GOERTZEL_DONE, index );  // "Goertzel DFT thread: %zu   Done"

   ExitThread( 0 );
}


/// Initialize resources needed by the Goertzel DFT module
///
/// @return `TRUE` if successful.  `FALSE` if there was a problem.
BOOL goertzel_Init() {
   _ASSERTE( ghStartDFTevent == NULL );

   /// #### Function

   /// - Initialize #ghStartDFTevent (The trigger to start all of the Goertzel
   ///   DFT work threads) with CreateEventW
   ghStartDFTevent = CreateEventW(
      NULL,                            // Default security attributes
      TRUE,                            // Manually reset after all 8 threads run
      FALSE,                           // Initial state
      NULL );                          // Object name
   if ( ghStartDFTevent == NULL ) {
      RETURN_FATAL( IDS_GOERTZEL_FAILED_TO_CREATE_STARTDFT_HANDLE );  // "Failed to create a ghStartDFTevent event handle.  Exiting."
   }

   /// - Initialize #ghDoneDFTevents (The "Done" events for each Goertzel DFT
   ///   work thread) with CreateEventW
   for ( int i = 0 ; i < NUMBER_OF_DTMF_TONES ; i++ ) {
      _ASSERTE( ghDoneDFTevents[ i ] == NULL );

      ghDoneDFTevents[ i ] = CreateEventW(
         NULL,                         // Default security attributes
         FALSE,                        // Automatically reset after the worker starts
         FALSE,                        // Initial state
         NULL );                       // Object name
      if ( ghDoneDFTevents[ i ] == NULL ) {
         RETURN_FATAL( IDS_GOERTZEL_FAILED_TO_CREATE_DONEDFT_HANDLES );  // "Failed to create a ghDoneDFTevents event handle.  Exiting."
      }
   }

   return TRUE;
}


/// Start the DFT work threads
///
/// @param  iSampleRate  Samples per second
/// @return `TRUE` if successful.  `FALSE` if there was a problem.
BOOL goertzel_Start( _In_ const int iSampleRate ) {
   _ASSERTE( iSampleRate > 0 );
   _ASSERTE( gstQueueSize > 0 );  // Set in #audioInit
   _ASSERTE( ghStartDFTevent != NULL );

   float floatSamplingRate = (float) iSampleRate;

   float floatNumSamples = (float) gstQueueSize;

   /// #### Function
   ///
   /// - Initialize values needed by the Goertzel DFT.  These values change with
   ///   every audio device, so we will set/reset them in the start/stop routines.
   sfScaleFactor = gstQueueSize / 2.0f;

   /// - Set sine, cosine and coeff for each DTMF tone in #gDtmfTones.
   for ( int i = 0 ; i < NUMBER_OF_DTMF_TONES ; i++ ) {
      int   k      = (int) ( 0.5f + ( ( floatNumSamples * gDtmfTones[ i ].frequency ) / (float) floatSamplingRate ) );
      float omega  = ( 2.0f * M_PIF * k ) / floatNumSamples;

      gDtmfTones[ i ].sine   = sinf( omega );
      gDtmfTones[ i ].cosine = cosf( omega );
      gDtmfTones[ i ].coeff  = 2.0f * gDtmfTones[ i ].cosine;
   }

   for ( int i = 0 ; i < NUMBER_OF_DTMF_TONES ; i++ ) {
      _ASSERTE( ghDoneDFTevents[ i ] != NULL );
      _ASSERTE( shWorkThreads[ i ] == NULL );

      /// - Use CreateThread to start the threads, storing the handles in #shWorkThreads
      shWorkThreads[i] = CreateThread(NULL, 0, goertzelWorkThread, &gDtmfTones[i].index, 0, NULL);
      if ( shWorkThreads[ i ] == NULL ) {
         RETURN_FATAL( IDS_GOERTZEL_FAILED_TO_CREATE_WORK_THREAD, i );  // "Failed to create Goertzel work thread %d.  Exiting."
      }
   }

   return TRUE;
}


/// Shutdown the Goertzel DFT work threads.
///
/// This function should not return until all of the Goertzel work threads have
/// stopped.
///
/// In Win32, threads will set their signalled state when they terminate.
/// Let's take advantage of that.
///
/// @see https://learn.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-setevent
/// @see https://learn.microsoft.com/en-us/windows/win32/sync/wait-functions
///
/// @return `TRUE` if successful.  `FALSE` if there was a problem.
BOOL goertzel_Stop() {
   /// #### Function
   ///
   /// - Set #gbIsRunning to `FALSE` -- just to be sure
   gbIsRunning = false;

   BOOL br;  // BOOL result

   /// - See how many theads are actually running
   int numRunningThreads = 0;
   for ( int i = 0 ; i < NUMBER_OF_DTMF_TONES ; i++ ) {
      if ( shWorkThreads[ i ] != NULL ) {
         numRunningThreads += 1;
      }
   }

   /// - If the threads are already stopped, then quietly return `TRUE`.
   if ( numRunningThreads == 0 ) {
      return TRUE;
   }

   _ASSERTE( ghStartDFTevent != NULL );
   for ( int i = 0 ; i < NUMBER_OF_DTMF_TONES ; i++ ) {
      _ASSERTE( ghDoneDFTevents[ i ] != NULL );
      // _ASSERTE( shWorkThreads[ i ] != NULL );  // Some, but not all threads may have already stopped
   }


   /// - Trigger all of the threads to run with SetEvent.  This will spin
   ///   their loops... with #gbIsRunning `== FALSE`, all of the threads will
   ///   then terminate.
   br = SetEvent( ghStartDFTevent );
   CHECK_BR_R( IDS_GOERTZEL_FAILED_TO_SIGNAL_START_DFT );  // "Failed to signal a ghStartDFTevent.  Exiting."

   /// - Wait for the threads to terminate with WaitForMultipleObjects
   ///     - If `bWaitAll` is `TRUE`, a return value within the specified range
   ///       indicates that the state of all specified objects are signaled.
   ///         - Read about WaitForMultipleObjects for the details.
   DWORD   dwWaitResult;  // Result from WaitForMultipleObjects
   dwWaitResult = WaitForMultipleObjects(
      NUMBER_OF_DTMF_TONES,  // Number of object handles
      ghDoneDFTevents,       // Array of object handles
      TRUE,                  // bWaitAll:  If TRUE, return when all objects are signaled.  If FALSE, return when any one of the objects are signaled.
      INFINITE );            // Time-out interval, in milliseconds
   if ( !( dwWaitResult >= WAIT_OBJECT_0 && dwWaitResult <= ( WAIT_OBJECT_0 + NUMBER_OF_DTMF_TONES - 1 ) ) ) {
      RETURN_FATAL( IDS_GOERTZEL_THREAD_END_FAILED );  // "Wait for all Goertzel threads to end failed.  Exiting."
   }

   /// - Cleanup the work thread handles
   for ( int i = 0 ; i < NUMBER_OF_DTMF_TONES ; i++ ) {
      shWorkThreads[ i ] = NULL;
   }

   LOG_TRACE_R( IDS_GOERTZEL_ENDED_NORMALLY );  // "All Goertzel threads ended normally."

   return TRUE;
}


/// Release resources used by the Goertzel DFT module
///
/// @return `TRUE` if successful.  `FALSE` if there was a problem.
BOOL goertzel_Release() {
   /// #### Function
   /// - Assert that the DFT work threads are not running
   for ( int i = 0 ; i < NUMBER_OF_DTMF_TONES ; i++ ) {
      _ASSERTE( shWorkThreads[ i ] == NULL );
   }

   BOOL br;  // BOOL result

   /// - Close all #ghDoneDFTevents with CloseHandle
   for ( int i = 0 ; i < NUMBER_OF_DTMF_TONES ; i++ ) {
      if ( ghDoneDFTevents[ i ] != NULL ) {
         br = CloseHandle( ghDoneDFTevents[ i ] );
         CHECK_BR_R( IDS_GOERTZEL_FAILED_TO_CLOSE_DONEDFT_HANDLES, i );  // "Failed to create a ghDoneDFTevents handle %d.  Exiting."
         ghDoneDFTevents[ i ] = NULL;
      }
   }

   /// - Close #ghStartDFTevent with CloseHandle
   if ( ghStartDFTevent != NULL ) {
      br = CloseHandle( ghStartDFTevent );
      CHECK_BR_R( IDS_GOERTZEL_FAILED_TO_CLOSE_STARTDFT_HANDLE );  //  "Failed to close ghStartDFTevent handle."
      ghStartDFTevent = NULL;
   }

   return TRUE;
}
