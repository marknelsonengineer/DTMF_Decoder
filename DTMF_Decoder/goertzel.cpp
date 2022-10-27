///////////////////////////////////////////////////////////////////////////////
//          University of Hawaii, College of Engineering
//          DTMF_Decoder - EE 469 - Fall 2022
//
//  A Windows Desktop C program that decodes DTMF tones
//
/// An 8-way multi-threaded Discrete Fast Forier Transform - specifically, 
/// the Goertzel algorithm for 8-bit PCM data.
/// 
/// @file goertzel.cpp
/// @version 1.0
///
/// @see https://github.com/Harvie/Programs/blob/master/c/goertzel/goertzel.c
/// @see https://en.wikipedia.org/wiki/Goertzel_algorithm
/// 
/// @author Mark Nelson <marknels@hawaii.edu>
/// @date   10_Oct_2022
///////////////////////////////////////////////////////////////////////////////

#include "framework.h"    // Standard system include files
#include "DTMF_Decoder.h" // For APP_NAME
#include "mvcModel.h"     // For pcmQueue and friends
#include "audio.h"        // For getSamplesPerSecond()
#include <avrt.h>         // For AvSetMmThreadCharacteristics()
#include <stdio.h>        // For sprintf_s()
#include "goertzel.h"     // For yo bad self


HANDLE startDFTevent[ NUMBER_OF_DTMF_TONES ];  ///< Handles to events
HANDLE doneDFTevent[ NUMBER_OF_DTMF_TONES ];   ///< Handles to events
HANDLE hWorkThreads[ NUMBER_OF_DTMF_TONES ];   ///< The worker threads

extern "C" float gfScaleFactor = 0;  ///< Set in goertzel_init() and used in goertzel_magnitude()

#ifdef _WIN64
   #pragma message( "Compiling 64-bit program" )

   extern "C" {
      void goertzel_magnitude_64( UINT8 index, dtmfTones_t* toneStruct );
   };
#else
   #pragma message( "Compiling 32-bit program" )
#endif


/// Compute the Goertzel magnitude of 8-bit PCM data
/// 
/// This 1-pass loop over #pcmQueue has been optimized for performance as it is 
/// processing audio data in realtime.
/// 
/// The original version of this algorithm came from:
/// @see https://github.com/Harvie/Programs/blob/master/c/goertzel/goertzel.c
///
/// @param index       The index into the DTMF tones array
/// @param toneStruct  A pointer to #dtmfTones (so it doesn't have to
///                    re-compute the index each time
void goertzel_magnitude( UINT8 index, dtmfTones_t* toneStruct ) {
   float real, imag;

   float q1 = 0;
   float q2 = 0;

   size_t queueRead = queueHead;  // Thread safe method to point to the next available byte for reading

   for ( size_t i = 0; i < queueSize; i++ ) {
      float q0 = toneStruct->coeff * q1 - q2 + ( (float) pcmQueue[ queueRead++ ] );
      q2 = q1;
      q1 = q0;
      if ( queueRead >= queueSize ) { // Wrap around at the end of the queue
         queueRead = 0;
      }
   }

   // Calculate the real and imaginary results scaling appropriately
   real = ( q1 * toneStruct->cosine - q2 );
   imag = ( q1 * toneStruct->sine );

   toneStruct->goertzelMagnitude = sqrtf( real * real + imag * imag ) / gfScaleFactor;
}


/// Runs one of the 8 DFT worker threads
/// 
/// @param Context Holds the index of which tone this thread is responsibile
///                for values are `0` through `7`.  This is critical for
///                thread safety.
/// 
/// @return `true` if successful.  `false` if there was a problem.
DWORD WINAPI goertzelWorkThread( LPVOID Context ) {
   CHAR sBuf[ 256 ];  // Debug buffer   /// @todo put a guard around this

   int index = *(int*) Context;
   
   sprintf_s( sBuf, sizeof( sBuf ), __FUNCTION__ ":  Start Goertzel DFT thread index=%d", index );
   OutputDebugStringA( sBuf );

   HANDLE mmcssHandle = NULL;  // Local to the thread for safety

   /// Set the multimedia class scheduler service, which will set the CPU
   /// priority for this thread
   /// 
   /// Note:  Uses the exported &mmcssTaskIndex that was originally set in audio.cpp
   /// 
   /// @see https://learn.microsoft.com/en-us/windows/win32/api/avrt/nf-avrt-avsetmmthreadcharacteristicsa
   mmcssHandle = AvSetMmThreadCharacteristics( L"Capture", &mmcssTaskIndex );
   if ( mmcssHandle == NULL ) {
      OutputDebugStringA( __FUNCTION__ ":  Failed to set MMCSS on Goertzel work thread.  Continuing." );
   }
   // OutputDebugStringA( __FUNCTION__ ":  Set MMCSS on Goertzel work thread." );

// while ( isRunning && index == 4 ) {   // Use for debugging/development
   while ( isRunning ) {
      DWORD dwWaitResult;

      /// Wait for this thread's #startDFTevent to be signalled
      dwWaitResult = WaitForSingleObject( startDFTevent[index], INFINITE);
      if ( dwWaitResult == WAIT_OBJECT_0 ) {
         if ( isRunning ) {
            #ifdef _WIN64
               goertzel_magnitude_64( index, &dtmfTones[ index ] );
            #else 
              goertzel_magnitude( index, &dtmfTones[index] );
            #endif

            if ( dtmfTones[index].goertzelMagnitude >= GOERTZEL_MAGNITUDE_THRESHOLD ) {
               mvcModelToggleToneDetectedStatus( index, true );
            } else {
               mvcModelToggleToneDetectedStatus( index, false );
            }

            SetEvent( doneDFTevent[ index ] );
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

   // The thread is done.  Shut it down.
   if ( mmcssHandle != NULL ) {
      if( !AvRevertMmThreadCharacteristics( mmcssHandle ) ) {
         OutputDebugStringA( __FUNCTION__ ":  Failed to revert MMCSS on Goertzel work thread.  Continuing." );
      }
      mmcssHandle = NULL;
   }

   OutputDebugStringA( __FUNCTION__ ":  End Goertzel DFT thread" );

   ExitThread( 0 );
}


/// Define PI as a floating point number
#define M_PIF 3.141592653589793238462643383279502884e+00F


/// Initialize values needed by the Goertzel DFT
/// 
/// @param  intSamplingRateParm  Samples per second
/// @return `true` if successful.  `false` if there was a problem.
BOOL goertzel_init( int intSamplingRateParm ) {
   float floatSamplingRate = (float) intSamplingRateParm;

   float floatnumSamples = (float) queueSize;

   gfScaleFactor = queueSize / 2.0f;

   /// Set sine, cosine and coeff for each DTMF tone in #dtmfTones.
   for ( int i = 0 ; i < NUMBER_OF_DTMF_TONES ; i++ ) {
      int   k      = (int) ( 0.5f + ( ( floatnumSamples * dtmfTones[ i ].frequency ) / (float) floatSamplingRate ) );
      float omega  = ( 2.0f * M_PIF * k ) / floatnumSamples;

      dtmfTones[ i ].sine   = sinf( omega );
      dtmfTones[ i ].cosine = cosf( omega );
      dtmfTones[ i ].coeff  = 2.0f * dtmfTones[ i ].cosine;
   }
 
   /// Create the events for synchronizing the threads
   for ( int i = 0 ; i < NUMBER_OF_DTMF_TONES ; i++ ) {
      startDFTevent[ i ] = CreateEventA( NULL, FALSE, FALSE, NULL );
      if ( startDFTevent[ i ] == NULL ) {
         OutputDebugStringA( __FUNCTION__ ":  Failed to create a startDFTevent event handle" );
         return FALSE;
      }

      doneDFTevent[ i ] = CreateEventA( NULL, FALSE, FALSE, NULL );
      if ( doneDFTevent[ i ] == NULL ) {
         OutputDebugStringA( __FUNCTION__ ":  Failed to create a doneDFTevent event handle" );
         return FALSE;
      }

      /// Start the threads
      hWorkThreads[i] = CreateThread(NULL, 0, goertzelWorkThread, &dtmfTones[i].index, 0, NULL);
      if ( hWorkThreads[ i ] == NULL ) {
         OutputDebugStringA( __FUNCTION__ ":  Failed to create a Goertzel work thread" );
         return FALSE;
      }
   }

   return TRUE;
}


/// Signal all of the threads so they can end on their own terms
/// 
/// @see https://learn.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-setevent
/// 
/// @return `true` if successful.  `false` if there was a problem.
BOOL goertzel_end() {
   isRunning = false;  // Just to be sure

   BOOL br;  // BOOL result

   for ( int i = 0 ; i < NUMBER_OF_DTMF_TONES ; i++ ) {
      // Trigger all of the threads to run... and spin their loops
      br = SetEvent( startDFTevent[ i ] );
      CHECK_BOOL_RESULT( "Failed to signal a startDFTevent" );

      br = SetEvent( doneDFTevent[ i ] );
      CHECK_BOOL_RESULT( "Failed to signal a doneDFTevent" );
   }

   return TRUE;
}


/// Cleanup Goertzel event handles and threads
/// 
/// @return `true` if successful.  `false` if there was a problem.
BOOL goertzel_cleanup() {
   BOOL br;  // BOOL result

   for ( int i = 0 ; i < NUMBER_OF_DTMF_TONES ; i++ ) {
      hWorkThreads[ i ] = NULL;

      br = CloseHandle( startDFTevent[ i ] );
      CHECK_BOOL_RESULT( "Failed to close startDFTevent handle" );
      startDFTevent[ i ] = NULL;

      CloseHandle( doneDFTevent[ i ] );
      CHECK_BOOL_RESULT( "Failed to close doneDFTevent handle" );
      doneDFTevent[ i ] = NULL;
   }
}


/// Signal the 8 Goertzel worker threads, then wait for them to finish their
/// analysis
/// 
/// @return `true` if successful.  `false` if there was a problem.
BOOL goertzel_compute_dtmf_tones() {

   for ( UINT8 i = 0 ; i < NUMBER_OF_DTMF_TONES ; i++ ) {
      /// Start each of the worker threads
      SetEvent( startDFTevent[i] );
   }

   /// Wait for all of the worker threads to signal their doneDFTevent
   WaitForMultipleObjects( NUMBER_OF_DTMF_TONES, doneDFTevent, TRUE, INFINITE );

   return TRUE;
}
