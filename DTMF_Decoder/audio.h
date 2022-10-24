///////////////////////////////////////////////////////////////////////////////
//          University of Hawaii, College of Engineering
//          DTMF_Decoder - EE 469 - Fall 2022
//
/// A Windows Desktop C program that decodes DTMF tones
/// 
/// @file audio.h
/// @version 1.0
///
/// @author Mark Nelson <marknels@hawaii.edu>
/// @date   10_Oct_2022
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <Windows.h>

extern BOOL initAudioDevice( HWND );
extern BOOL stopAudioDevice( HWND );
extern BOOL cleanupAudioDevice();

extern DWORD getSamplesPerSecond();   // TODO: This may be unused.  Get rid of it
 
#define PCM_8_BIT_SILENCE 127     /* Silence is 127 */

extern size_t queueSize;   
