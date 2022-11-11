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
/// | `WaitForSingleObject`             | https://learn.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-waitforsingleobject     |
/// | `WaitForMultipleObjects`          | https://learn.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-waitformultipleobjects  |
/// | `CreateEventA`                    | https://learn.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-createeventa            |
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
///
/// @file    goertzel.cpp
/// @author  Mark Nelson <marknels@hawaii.edu>
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


       HANDLE ghStartDFTevent[ NUMBER_OF_DTMF_TONES ];  ///< Handles to events.  External to support inlining.
       HANDLE ghDoneDFTevent[ NUMBER_OF_DTMF_TONES ];   ///< Handles to events.  External to support inlining.
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
/// Inlined for performance reasons.
///
/// @param index       The index into the DTMF tones array
/// @param toneStruct  A pointer to #gDtmfTones (so it doesn't have to
///                    re-compute the index each time
__forceinline static void goertzel_magnitude(
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
   _ASSERTE( ghStartDFTevent[ index ] != NULL );
   _ASSERTE( ghDoneDFTevent[ index ]  != NULL );

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

      /// Wait for this thread's #ghStartDFTevent to be signalled
      dwWaitResult = WaitForSingleObject( ghStartDFTevent[index], INFINITE);
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

            SetEvent( ghDoneDFTevent[ index ] );
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
      ghStartDFTevent[ i ] = CreateEventA( NULL, FALSE, FALSE, NULL );
      if ( ghStartDFTevent[ i ] == NULL ) {
         LOG_ERROR( "Failed to create a startDFTevent event handle" );
         return FALSE;
      }

      ghDoneDFTevent[ i ] = CreateEventA( NULL, FALSE, FALSE, NULL );
      if ( ghDoneDFTevent[ i ] == NULL ) {
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
      br = SetEvent( ghStartDFTevent[ i ] );
      CHECK_BR( "Failed to signal a startDFTevent" );

      br = SetEvent( ghDoneDFTevent[ i ] );
      CHECK_BR( "Failed to signal a doneDFTevent" );
   }

   return TRUE;
}


/// Cleanup Goertzel event handles and threads
///
/// @return `TRUE` if successful.  `FALSE` if there was a problem.
BOOL goertzel_cleanup() {
   BOOL br;  // BOOL result

   for ( int i = 0 ; i < NUMBER_OF_DTMF_TONES ; i++ ) {
      shWorkThreads[ i ] = NULL;

      if ( ghStartDFTevent[ i ] != NULL ) {
         br = CloseHandle( ghStartDFTevent[ i ] );
         CHECK_BR( "Failed to close ghStartDFTevent handle" );
         ghStartDFTevent[ i ] = NULL;
      }

      if ( ghDoneDFTevent[ i ] != NULL ) {
         br = CloseHandle( ghDoneDFTevent[ i ] );
         CHECK_BR( "Failed to close ghDoneDFTevent handle" );
         ghDoneDFTevent[ i ] = NULL;
      }
   }

   return TRUE;
}
