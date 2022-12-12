///////////////////////////////////////////////////////////////////////////////
//          University of Hawaii, College of Engineering
//          DTMF_Decoder - EE 469 - Fall 2022
//
//  A Windows Desktop C program that decodes DTMF tones
//
/// Initialize and process audio from the default Windows audio capture device
///
/// @file    audio.h
/// @author  Mark Nelson <marknels@hawaii.edu>
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <Windows.h>  // For BOOL


extern BOOL audioInit();
extern BOOL audioStart();
extern BOOL audioStop();
extern BOOL audioCleanup();


/// The definition of PCM silence is 127 (in this prgram)
#define PCM_8_BIT_SILENCE 127     /* Silence is 127 */
