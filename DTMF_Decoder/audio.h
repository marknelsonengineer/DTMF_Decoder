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

BOOL initAudioDevice( HWND );
BOOL cleanupAudioDevice();

BOOL startAudioStream( HWND );
BOOL endAudioStream( HWND );
