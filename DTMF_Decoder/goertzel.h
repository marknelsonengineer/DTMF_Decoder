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
/// @file    goertzel.h
/// @author  Mark Nelson <marknels@hawaii.edu>
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <Windows.h>      // For BOOL, etc.


/// When the result of #goertzel_magnitude `>=` this, then this means it's
/// detected a tone.
#define GOERTZEL_MAGNITUDE_THRESHOLD  10.0f

extern BOOL goertzel_init( _In_ const int SAMPLING_RATE_IN );

extern BOOL goertzel_end();

extern BOOL goertzel_cleanup();

extern HANDLE ghStartDFTevent[ NUMBER_OF_DTMF_TONES ];
extern HANDLE ghDoneDFTevent[ NUMBER_OF_DTMF_TONES ];

/// Signal the 8 Goertzel worker threads, then wait for them to finish their
/// analysis
///
/// Inlined for performance
///
/// @return `true` if successful.  `false` if there was a problem.
__forceinline BOOL goertzel_compute_dtmf_tones() {
   BOOL    br;            // BOOL result
   DWORD   dwWaitResult;  // Result from WaitForMultipleObjects

   /// Start each of the worker threads (loop unrolled for performance)
   br = SetEvent( ghStartDFTevent[ 0 ] );
   CHECK_BR( "Failed to start a DFT worker thread" );
   br = SetEvent( ghStartDFTevent[ 1 ] );
   CHECK_BR( "Failed to start a DFT worker thread" );
   br = SetEvent( ghStartDFTevent[ 2 ] );
   CHECK_BR( "Failed to start a DFT worker thread" );
   br = SetEvent( ghStartDFTevent[ 3 ] );
   CHECK_BR( "Failed to start a DFT worker thread" );
   br = SetEvent( ghStartDFTevent[ 4 ] );
   CHECK_BR( "Failed to start a DFT worker thread" );
   br = SetEvent( ghStartDFTevent[ 5 ] );
   CHECK_BR( "Failed to start a DFT worker thread" );
   br = SetEvent( ghStartDFTevent[ 6 ] );
   CHECK_BR( "Failed to start a DFT worker thread" );
   br = SetEvent( ghStartDFTevent[ 7 ] );
   CHECK_BR( "Failed to start a DFT worker thread" );

   /// Wait for all of the worker threads to signal their ghDoneDFTevent
   dwWaitResult = WaitForMultipleObjects( NUMBER_OF_DTMF_TONES, ghDoneDFTevent, TRUE, INFINITE );

   /// For performance reasons, I'm asserting the result of the `WaitForMultipleObjects`.
   /// I don't want to compute this in the Release version for each audio buffer run.
   _ASSERTE( dwWaitResult >= WAIT_OBJECT_0 && dwWaitResult <= WAIT_OBJECT_0 + NUMBER_OF_DTMF_TONES - 1 );

   return TRUE;
}
