///////////////////////////////////////////////////////////////////////////////
//          University of Hawaii, College of Engineering
//          DTMF_Decoder - EE 469 - Fall 2022
//
/// A Windows Desktop C program that decodes DTMF tones
///
/// @file goertzel.h
/// @version 1.0
///
/// @author Mark Nelson <marknels@hawaii.edu>
/// @date   10_Oct_2022
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <stdint.h>
#include <windows.h>


extern float goertzel_magnitude( int numSamples, float TARGET_FREQUENCY, int SAMPLING_RATE ) ;

extern BOOL compute_dtmf_tones_with_goertzel();
