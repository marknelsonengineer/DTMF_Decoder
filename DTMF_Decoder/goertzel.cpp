///////////////////////////////////////////////////////////////////////////////
//          University of Hawaii, College of Engineering
//          DTMF_Decoder22X!Vqrpp1kz9C!ma3mCkbd - EE 469 - Fall 2022
//
/// A Windows Desktop C program that decodes DTMF tones
/// 
/// @file goertzel.cpp
/// @version 1.0
///
/// @author Mark Nelson <marknels@hawaii.edu>
/// @date   10_Oct_2022
///////////////////////////////////////////////////////////////////////////////

#include "framework.h"
#include "DTMF_Decoder.h" 
#include "mvcModel.h"
#include <stdint.h>
#define _USE_MATH_DEFINES // for C++  
#include <math.h>
#include "audio.h"  // for getSamplesPerSecond()
#include <avrt.h>    // For AvSetMmThreadCharacteristics()
#include <stdio.h>
#include "goertzel.h" 


// int numSamples = 0;
int SAMPLING_RATE = 0;
float   floatnumSamples = 0;
float   scalingFactor = 0;

HANDLE startDFTevent[ NUMBER_OF_DTMF_TONES ];
HANDLE doneDFTevent[ NUMBER_OF_DTMF_TONES ];
HANDLE hWorkThreads[ NUMBER_OF_DTMF_TONES ];


void goertzel_magnitude( UINT8 index ) {
   float real, imag;

   float q1 = 0;
   float q2 = 0;

   size_t queueRead = queueHead;  // Thread safe method to point to the next available byte for reading

   for ( size_t i = 0; i < queueSize; i++ ) {
      float q0 = dtmfTones[ index ].coeff * q1 - q2 + ( (float) pcmQueue[ queueRead++ ] );
      q2 = q1;
      q1 = q0;
      queueRead %= queueSize;  // TODO:  There are probably more clever/efficient ways to do this, but this is very clear
   }

   // calculate the real and imaginary results
   // scaling appropriately
   real = ( q1 * dtmfTones[ index ].cosine - q2 );
   imag = ( q1 * dtmfTones[ index ].sine );

   dtmfTones[ index ].goertzelMagnitude = sqrtf( real * real + imag * imag ) / scalingFactor;

   float threshold = THRESHOLD;

   if ( dtmfTones[ index ].goertzelMagnitude >= threshold ) {
      editToneDetectedStatus( index, true );
   } else {
      editToneDetectedStatus( index, false );
   }

   SetEvent( doneDFTevent[ index ] );
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
            goertzel_magnitude( index );
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

BOOL goertzel_init( int SAMPLING_RATE_IN ) {
   // numSamples = numSamplesIn;
   SAMPLING_RATE = SAMPLING_RATE_IN;

   floatnumSamples = (float) queueSize;

   scalingFactor = queueSize / 2.0f;

   int     k;

   float   omega, sine, cosine, coeff;

   for ( int i = 0 ; i < NUMBER_OF_DTMF_TONES ; i++ ) {
      k = (int) ( 0.5f + ( ( floatnumSamples * dtmfTones[ i ].frequency ) / (float) SAMPLING_RATE ) );
      omega = ( 2.0f * M_PIF * k ) / floatnumSamples;
      sine = sinf( omega );
      cosine = cosf( omega );
      coeff = 2.0f * cosine;

      dtmfTones[ i ].sine = sine;
      dtmfTones[ i ].cosine = cosine;
      dtmfTones[ i ].coeff = coeff;
   }
 

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

         /// Start the thread
      hWorkThreads[i] = CreateThread(NULL, 0, goertzelWorkThread, &dtmfTones[i].index, 0, NULL);
      if ( hWorkThreads[ i ] == NULL ) {
         OutputDebugStringA( __FUNCTION__ ":  Failed to create a Goertzel work thread" );
         return FALSE;
      }

   }

   return TRUE;
}


void goertzel_end() {
   isRunning = false;  // Just to be sure

   for ( int i = 0 ; i < NUMBER_OF_DTMF_TONES ; i++ ) {
      // Trigger all of the threads to run... and spin their loops
      SetEvent( startDFTevent[ i ] );
      SetEvent( doneDFTevent[ i ] );
   }
}


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



BOOL compute_dtmf_tones_with_goertzel() {

   for ( UINT8 i = 0 ; i < NUMBER_OF_DTMF_TONES ; i++ ) {
      SetEvent( startDFTevent[i] );
   }

   WaitForMultipleObjects( NUMBER_OF_DTMF_TONES, doneDFTevent, TRUE, INFINITE );

   return TRUE;
}
