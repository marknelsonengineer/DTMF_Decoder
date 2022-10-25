///////////////////////////////////////////////////////////////////////////////
//          University of Hawaii, College of Engineering
//          DTMF_Decoder - EE 469 - Fall 2022
//
/// A Windows Desktop C program that decodes DTMF tones
///
/// An 8-way multi-threaded Discrete Fast Forier Transform - specifically, 
/// the Goertzel algorithm for 8-bit PCM data.
/// 
/// @file goertzel.h
/// @version 1.0
///
/// @see https://github.com/Harvie/Programs/blob/master/c/goertzel/goertzel.c
/// @see https://en.wikipedia.org/wiki/Goertzel_algorithm
///
/// @author Mark Nelson <marknels@hawaii.edu>
/// @date   10_Oct_2022
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <stdint.h>
#include <windows.h>

#define GOERTZEL_MAGNITUDE_THRESHOLD  3.0f

extern BOOL goertzel_init( int SAMPLING_RATE_IN );  /// Initialize this module

extern BOOL goertzel_compute_dtmf_tones(); /// Signal 8 worker threads, then 
                                           ///wait for them to end
extern void goertzel_end();                /// Signal all of the threads so 
                                           /// they can end on their own terms
void        goertzel_cleanup();            /// Cleanup handles and threads       
