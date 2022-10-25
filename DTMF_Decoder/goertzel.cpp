///////////////////////////////////////////////////////////////////////////////
//          University of Hawaii, College of Engineering
//          DTMF_Decoder - EE 469 - Fall 2022
//
/// A Windows Desktop C program that decodes DTMF tones
///
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

#include "framework.h"
#include "mvcModel.h"     // For pcmQueue and friends
#define _USE_MATH_DEFINES // For C++ (this is a .cpp file) 
#include <math.h>         // For sinf() and cosf()
#include "audio.h"        // For getSamplesPerSecond()
#include <avrt.h>         // For AvSetMmThreadCharacteristics()
#include <stdio.h>        // For sprintf_s()
#include "goertzel.h"     // For yo bad self


HANDLE startDFTevent[ NUMBER_OF_DTMF_TONES ];  /// Handles to events
HANDLE doneDFTevent[ NUMBER_OF_DTMF_TONES ];   /// Handles to events
HANDLE hWorkThreads[ NUMBER_OF_DTMF_TONES ];   /// The worker threads

static float gfScaleFactor = 0;  /// Set in goertzel_init() and used in goertzel_magnitude()


/// Compute the Goertzel magnitude of 8-bit PCM data
/// 
/// This 1-pass loop over the pcmQueue has been optimized for performance as it is 
/// processing audio data in realtime.
/// 
/// The original version of this algorithm came from:
/// @see https://github.com/Harvie/Programs/blob/master/c/goertzel/goertzel.c
///
/// @param index       The index into the DTMF tones array
/// @param toneStruct  A pointer to the DTMF tones structure (so it doesn't have to
///                    re-multiply the index each time
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

   // calculate the real and imaginary results
   // scaling appropriately
   real = ( q1 * toneStruct->cosine - q2 );
   imag = ( q1 * toneStruct->sine );

   toneStruct->goertzelMagnitude = sqrtf( real * real + imag * imag ) / gfScaleFactor;
}


DWORD WINAPI goertzelWorkThread( LPVOID Context ) {
   CHAR sBuf[ 256 ];  // Debug buffer   // TODO: put a guard around this
   int index = *(int*) Context;
   sprintf_s( sBuf, sizeof( sBuf ), __FUNCTION__ ":  Start Goertzel DFT thread index=%d", index );
   OutputDebugStringA( sBuf );

   HANDLE mmcssHandle = NULL;
   DWORD mmcssTaskIndex = 0;

   /// Set the multimedia class scheduler service, which will set the CPU
   /// priority for this thread
   mmcssHandle = AvSetMmThreadCharacteristics( L"Capture", &mmcssTaskIndex );
   if ( mmcssHandle == NULL ) {
      OutputDebugStringA( __FUNCTION__ ":  Failed to set MMCSS on Goertzel work thread.  Continuing." );
   }
   OutputDebugStringA( __FUNCTION__ ":  Set MMCSS on Goertzel work thread." );

   while ( isRunning ) {
      DWORD dwWaitResult;

      dwWaitResult = WaitForSingleObject( startDFTevent[index], INFINITE);
      if ( dwWaitResult == WAIT_OBJECT_0 ) {
         if ( isRunning ) {
            goertzel_magnitude( index, &dtmfTones[index] );

            if ( dtmfTones[index].goertzelMagnitude >= GOERTZEL_MAGNITUDE_THRESHOLD ) {
               editToneDetectedStatus( index, true );
            } else {
               editToneDetectedStatus( index, false );
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

   OutputDebugStringA( __FUNCTION__ ":  End Goertzel DFT thread" );

   ExitThread( 0 );
}


#define M_PIF 3.141592653589793238462643383279502884e+00F

BOOL goertzel_init( int intSamplingRateParm ) {
   float floatSamplingRate = (float) intSamplingRateParm;

   float floatnumSamples = (float) queueSize;

   gfScaleFactor = queueSize / 2.0f;

   int     k;

   float   omega, sine, cosine, coeff;

   for ( int i = 0 ; i < NUMBER_OF_DTMF_TONES ; i++ ) {
      k = (int) ( 0.5f + ( ( floatnumSamples * dtmfTones[ i ].frequency ) / (float) floatSamplingRate ) );
      omega = ( 2.0f * M_PIF * k ) / floatnumSamples;
      sine = sinf( omega );
      cosine = cosf( omega );
      coeff = 2.0f * cosine;

      dtmfTones[ i ].sine = sine;
      dtmfTones[ i ].cosine = cosine;
      dtmfTones[ i ].coeff = coeff;
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


// TODO:  Return BOOL and then check it where it's called
void goertzel_end() {
   isRunning = false;  // Just to be sure

   for ( int i = 0 ; i < NUMBER_OF_DTMF_TONES ; i++ ) {
      // Trigger all of the threads to run... and spin their loops
      SetEvent( startDFTevent[ i ] );
      SetEvent( doneDFTevent[ i ] );
   }
}


// TODO:  Return BOOL and then check it where it's called
void goertzel_cleanup() {
   for ( int i = 0 ; i < NUMBER_OF_DTMF_TONES ; i++ ) {
      hWorkThreads[ i ] = NULL;

      CloseHandle( startDFTevent[ i ] );
      startDFTevent[ i ] = NULL;
      CloseHandle( doneDFTevent[ i ] );
      doneDFTevent[ i ] = NULL;

      // TODO: I probably need to cleanup AvSetMmThreadCharacteristics as well
   }
}


BOOL goertzel_compute_dtmf_tones() {

   for ( UINT8 i = 0 ; i < NUMBER_OF_DTMF_TONES ; i++ ) {
      /// Start each of the worker threads
      SetEvent( startDFTevent[i] );
   }

   /// Wait for all of the worker threads to signal their doneDFTevent
   WaitForMultipleObjects( NUMBER_OF_DTMF_TONES, doneDFTevent, TRUE, INFINITE );

   return TRUE;
}
