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


/// When the result of #goertzel_Magnitude `>=` this, then this means it's
/// detected a tone.
#define GOERTZEL_MAGNITUDE_THRESHOLD  10.0f

extern BOOL goertzel_Init();

extern BOOL goertzel_Start( _In_ const int SAMPLING_RATE_IN );

extern BOOL goertzel_Stop();

extern BOOL goertzel_Cleanup();

extern HANDLE ghStartDFTevent;
extern HANDLE ghDoneDFTevent[ NUMBER_OF_DTMF_TONES ];

/// Signal the Goertzel DFT worker threads to start, then wait for all 8 of
/// them to finish their analysis
///
/// Inlined for performance.
///
/// @return `TRUE` if successful.  `FALSE` if there was a problem.
__forceinline BOOL goertzel_compute_dtmf_tones() {
   BOOL  br;            // BOOL result
   DWORD dwWaitResult;  // Result from WaitForMultipleObjects

   /// Start all of the worker threads
   br = SetEvent( ghStartDFTevent );
   CHECK_BR_Q( IDS_GOERTZEL_FAILED_TO_SIGNAL_START_DFT, 0 );  // "Failed to signal a ghStartDFTevent.  Exiting."

   /// Wait for all of the worker threads to signal their ghDoneDFTevent
   dwWaitResult = WaitForMultipleObjects( NUMBER_OF_DTMF_TONES, ghDoneDFTevent, TRUE, INFINITE );

   /// For performance reasons, I'm asserting the result of the `WaitForMultipleObjects`.
   /// I don't want to compute this in the Release version for each audio buffer run.
   _ASSERTE( dwWaitResult >= WAIT_OBJECT_0 && dwWaitResult <= ( WAIT_OBJECT_0 + NUMBER_OF_DTMF_TONES - 1 ) );

   br = ResetEvent( ghStartDFTevent );
   CHECK_BR_Q( IDS_GOERTZEL_FAILED_TO_RESET_STARTDFT_EVENT, 0 );  // "Failed to reset the DFT start event"

   return TRUE;
}
