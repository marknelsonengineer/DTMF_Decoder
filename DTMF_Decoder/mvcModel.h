///////////////////////////////////////////////////////////////////////////////
//          University of Hawaii, College of Engineering
//          DTMF_Decoder - EE 469 - Fall 2022
//
/// A Windows Desktop C program that decodes DTMF tones
///
/// @file mvcModel.h
/// @version 1.0
///
/// @author Mark Nelson <marknels@hawaii.edu>
/// @date   10_Oct_2022
///////////////////////////////////////////////////////////////////////////////

#pragma once

typedef struct {
   float frequency;  /// The DTMF tone
   bool  detected;   /// True if the tone is present, false if it's not
} dtmfTones_t;

extern dtmfTones_t dtmfTones[ 8 ]; 

/// @TODO I should convert this to a read-only interface