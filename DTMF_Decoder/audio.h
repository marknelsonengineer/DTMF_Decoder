///////////////////////////////////////////////////////////////////////////////
//          University of Hawaii, College of Engineering
//          DTMF_Decoder - EE 469 - Fall 2022
//
//  A Windows Desktop C program that decodes DTMF tones
//
/// Windows Audio Driver code
/// 
/// @file audio.h
/// @version 1.0
///
/// @author Mark Nelson <marknels@hawaii.edu>
/// @date   10_Oct_2022
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <Windows.h>      // For BOOL, HWND, etc.


extern BOOL audioInit( HWND );
extern BOOL audioStopDevice( HWND );
extern BOOL audioCleanup();


/// The definition of PCM silence is 127 (in this prgram)
#define PCM_8_BIT_SILENCE 127     /* Silence is 127 */
