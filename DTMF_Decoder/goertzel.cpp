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
///
/// @see https://learn.microsoft.com/en-us/cpp/c-runtime-library/math-constants?view=msvc-170
/// @see https://learn.microsoft.com/en-us/cpp/preprocessor/message?view=msvc-170
/// @see https://learn.microsoft.com/en-us/windows/win32/api/avrt/nf-avrt-avsetmmthreadcharacteristicsa
/// @see https://learn.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-waitforsingleobject
/// @see https://learn.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-setevent
/// @see https://learn.microsoft.com/en-us/windows/win32/api/avrt/nf-avrt-avrevertmmthreadcharacteristics
/// @see https://learn.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-createeventa
/// @see https://learn.microsoft.com/en-us/windows/win32/api/handleapi/nf-handleapi-closehandle
/// @see https://learn.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-waitformultipleobjects
///
/// @see https://en.cppreference.com/w/c/numeric/math/sqrt
/// @see https://en.cppreference.com/w/c/numeric/math/sin
/// @see https://en.cppreference.com/w/c/numeric/math/cos
///
/// @file    goertzel.cpp
/// @version 1.0
///
/// @author  Mark Nelson <marknels@hawaii.edu>
/// @date    10_Oct_2022
///////////////////////////////////////////////////////////////////////////////

#include "framework.h"    // Standard system include files
#include <avrt.h>         // For AvSetMmThreadCharacteristics()
#include <stdio.h>        // For sprintf_s()

///< For getting math defines in C++ (this is a .cpp file)
#define _USE_MATH_DEFINES
#include <math.h>              // For sinf() and cosf()

#include "DTMF_Decoder.h" // For APP_NAME
#include "mvcModel.h"     // For gPcmQueue and friends
#include "goertzel.h"     // For yo bad self


static HANDLE shStartDFTevent[ NUMBER_OF_DTMF_TONES ];  ///< Handles to events
static HANDLE shDoneDFTevent[ NUMBER_OF_DTMF_TONES ];   ///< Handles to events
static HANDLE shWorkThreads[ NUMBER_OF_DTMF_TONES ];    ///< The worker threads

extern "C" float fScaleFactor = 0;  ///< Set in goertzel_init() and used in goertzel_magnitude()

#ifdef _WIN64
   #pragma message( "Compiling 64-bit program" )

   extern "C" {
      void goertzel_magnitude_64( const UINT8 index, dtmfTones_t* toneStruct );
   };
#else
   #pragma message( "Compiling 32-bit program" )
#endif


/// Compute the Goertzel magnitude of 8-bit PCM data
///
/// This 1-pass loop over #gPcmQueue has been optimized for performance as it is
/// processing audio data in realtime.
///
/// The original version of this algorithm came from:
/// @see https://github.com/Harvie/Programs/blob/master/c/goertzel/goertzel.c
///
/// @param index       The index into the DTMF tones array
/// @param toneStruct  A pointer to #gDtmfTones (so it doesn't have to
///                    re-compute the index each time
void goertzel_magnitude(
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


/// Runs one of the 8 DFT worker threads
///
/// @see https://learn.microsoft.com/en-us/previous-versions/windows/desktop/legacy/ms686736(v=vs.85)
///
/// @param pContext Holds the index of which tone this thread is responsibile
///                 for values are `0` through `7`.  This is critical for
///                 thread safety.
///
/// @return `true` if successful.  `false` if there was a problem.
DWORD WINAPI goertzelWorkThread( _In_ LPVOID pContext ) {
   _ASSERTE( pContext != NULL );

   int iIndex = *(int*) pContext;  // This comes to us as an int from dtmfTones_t
   size_t index = iIndex;          // But we use it as an index into an array, so convert to size_t

   _ASSERTE( iIndex < NUMBER_OF_DTMF_TONES );
   _ASSERTE( shStartDFTevent[ index ] != NULL );
   _ASSERTE( shDoneDFTevent[ index ]  != NULL );

   LOG_TRACE( "Start Goertzel DFT thread index=%zu", index );

   HANDLE mmcssHandle = NULL;  // Local to the thread for safety

   /// Set the multimedia class scheduler service, which will set the CPU
   /// priority for this thread
   ///
   /// Note:  Uses the exported &gdwMmcssTaskIndex that was originally set in audio.cpp
   ///
   /// @see https://learn.microsoft.com/en-us/windows/win32/api/avrt/nf-avrt-avsetmmthreadcharacteristicsa
   mmcssHandle = AvSetMmThreadCharacteristicsW( L"Capture", &gdwMmcssTaskIndex );
   if ( mmcssHandle == NULL ) {
      LOG_WARN( "Failed to set MMCSS on Goertzel work thread.  Continuing." );
   }
   // LOG_INFO( "Set MMCSS on Goertzel work thread." );

// while ( gbIsRunning && index == 4 ) {   // Use for debugging/development
   while ( gbIsRunning ) {
      DWORD dwWaitResult;

      /// Wait for this thread's #shStartDFTevent to be signalled
      dwWaitResult = WaitForSingleObject( shStartDFTevent[index], INFINITE);
      if ( dwWaitResult == WAIT_OBJECT_0 ) {
         if ( gbIsRunning ) {
            #ifdef _WIN64
               goertzel_magnitude_64( (UINT8) index, &gDtmfTones[index] );
            #else
              goertzel_magnitude( iIndex, &gDtmfTones[index] );
            #endif

            if ( gDtmfTones[index].goertzelMagnitude >= GOERTZEL_MAGNITUDE_THRESHOLD ) {
               mvcModelToggleToneDetectedStatus( index, true );
            } else {
               mvcModelToggleToneDetectedStatus( index, false );
            }

            SetEvent( shDoneDFTevent[ index ] );
         }
      } else if ( dwWaitResult == WAIT_FAILED ) {
         LOG_FATAL( "WaitForSingleObject in Goertzel thread failed.  Exiting.  Investigate!" );
         gracefulShutdown();
         break;  // While loop
      } else {
         LOG_FATAL( "WaitForSingleObject in Goertzel thread ended for an unknown reason.  Exiting.  Investigate!" );
         gracefulShutdown();
         break;  // While loop
      }
   }

   // The thread is done.  Shut it down.
   if ( mmcssHandle != NULL ) {
      if( !AvRevertMmThreadCharacteristics( mmcssHandle ) ) {
         LOG_WARN( "Failed to revert MMCSS on Goertzel work thread.  Continuing." );
      }
      mmcssHandle = NULL;
   }

   LOG_TRACE( "End Goertzel DFT thread." );

   ExitThread( 0 );
}


/// Define PI as a floating point number
#define M_PIF 3.141592653589793238462643383279502884e+00F


/// Initialize values needed by the Goertzel DFT
///
/// @param  iSampleRate  Samples per second
/// @return `true` if successful.  `false` if there was a problem.
BOOL goertzel_init( _In_ const int iSampleRate ) {
   _ASSERTE( iSampleRate > 0 );
   _ASSERTE( gstQueueSize > 0 );

   float floatSamplingRate = (float) iSampleRate;

   float floatnumSamples = (float) gstQueueSize;

   fScaleFactor = gstQueueSize / 2.0f;

   /// Set sine, cosine and coeff for each DTMF tone in #gDtmfTones.
   for ( int i = 0 ; i < NUMBER_OF_DTMF_TONES ; i++ ) {
      int   k      = (int) ( 0.5f + ( ( floatnumSamples * gDtmfTones[ i ].frequency ) / (float) floatSamplingRate ) );
      float omega  = ( 2.0f * M_PIF * k ) / floatnumSamples;

      gDtmfTones[ i ].sine   = sinf( omega );
      gDtmfTones[ i ].cosine = cosf( omega );
      gDtmfTones[ i ].coeff  = 2.0f * gDtmfTones[ i ].cosine;
   }

   /// Create the events for synchronizing the threads
   for ( int i = 0 ; i < NUMBER_OF_DTMF_TONES ; i++ ) {
      shStartDFTevent[ i ] = CreateEventA( NULL, FALSE, FALSE, NULL );
      if ( shStartDFTevent[ i ] == NULL ) {
         LOG_ERROR( "Failed to create a startDFTevent event handle" );
         return FALSE;
      }

      shDoneDFTevent[ i ] = CreateEventA( NULL, FALSE, FALSE, NULL );
      if ( shDoneDFTevent[ i ] == NULL ) {
         LOG_ERROR( "Failed to create a doneDFTevent event handle" );
         return FALSE;
      }

      /// Start the threads
      shWorkThreads[i] = CreateThread(NULL, 0, goertzelWorkThread, &gDtmfTones[i].index, 0, NULL);
      if ( shWorkThreads[ i ] == NULL ) {
         LOG_ERROR( "Failed to create a Goertzel work thread" );
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
   gbIsRunning = false;  // Just to be sure

   BOOL br;  // BOOL result

   for ( int i = 0 ; i < NUMBER_OF_DTMF_TONES ; i++ ) {
      // Trigger all of the threads to run... and spin their loops
      br = SetEvent( shStartDFTevent[ i ] );
      CHECK_BR( "Failed to signal a startDFTevent" );

      br = SetEvent( shDoneDFTevent[ i ] );
      CHECK_BR( "Failed to signal a doneDFTevent" );
   }

   return TRUE;
}


/// Cleanup Goertzel event handles and threads
///
/// @return `true` if successful.  `false` if there was a problem.
BOOL goertzel_cleanup() {
   BOOL br;  // BOOL result

   for ( int i = 0 ; i < NUMBER_OF_DTMF_TONES ; i++ ) {
      shWorkThreads[ i ] = NULL;

      br = CloseHandle( shStartDFTevent[ i ] );
      CHECK_BR( "Failed to close startDFTevent handle" );
      shStartDFTevent[ i ] = NULL;

      CloseHandle( shDoneDFTevent[ i ] );
      CHECK_BR( "Failed to close doneDFTevent handle" );
      shDoneDFTevent[ i ] = NULL;
   }

   return TRUE;
}


/// Signal the 8 Goertzel worker threads, then wait for them to finish their
/// analysis
///
/// @return `true` if successful.  `false` if there was a problem.
BOOL goertzel_compute_dtmf_tones() {
   BOOL    br;            // BOOL result
   DWORD   dwWaitResult;  // Result from WaitForMultipleObjects

   for ( UINT8 i = 0 ; i < NUMBER_OF_DTMF_TONES ; i++ ) {
      /// Start each of the worker threads
      br = SetEvent( shStartDFTevent[i] );
      CHECK_BR( "Failed to start a DFT worker thread" );
   }

   /// Wait for all of the worker threads to signal their shDoneDFTevent
   dwWaitResult = WaitForMultipleObjects( NUMBER_OF_DTMF_TONES, shDoneDFTevent, TRUE, INFINITE );

   /// For performance reasons, I'm asserting the result of the `WaitForMultipleObjects`.
   /// I don't want to compute this in the Release version for each audio buffer run.
   _ASSERTE( dwWaitResult >= WAIT_OBJECT_0 && dwWaitResult <= WAIT_OBJECT_0 + NUMBER_OF_DTMF_TONES - 1 );

   return TRUE;
}
