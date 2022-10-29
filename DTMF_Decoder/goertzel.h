///////////////////////////////////////////////////////////////////////////////
//          University of Hawaii, College of Engineering
//          DTMF_Decoder - EE 469 - Fall 2022
//
//  A Windows Desktop C program that decodes DTMF tones
//
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

#include <Windows.h>      // For BOOL, etc.


/// When the result of #goertzel_magnitude `>=` this, then this means it's 
/// detected a tone.
#define GOERTZEL_MAGNITUDE_THRESHOLD  10.0f

extern BOOL goertzel_init( int SAMPLING_RATE_IN );  

extern BOOL goertzel_compute_dtmf_tones();  
                                           
extern BOOL goertzel_end();                
                                           
extern BOOL goertzel_cleanup();                  
