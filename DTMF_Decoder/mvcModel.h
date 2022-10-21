///////////////////////////////////////////////////////////////////////////////
//          University of Hawaii, College of Engineering
//          DTMF_Decoder - EE 469 - Fall 2022
//
/// A Windows Desktop C program that decodes DTMF tones
///
/// The model will hold state between the controller and view code
/// 
/// @file mvcModel.h
/// @version 1.0
///
/// @author Mark Nelson <marknels@hawaii.edu>
/// @date   10_Oct_2022
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <windows.h>

typedef struct {
   float frequency;    /// The DTMF tone
   bool  detected;     /// True if the tone is present, false if it's not
   WCHAR label[ 16 ];  /// A label for the tone
} dtmfTones_t;

extern dtmfTones_t dtmfTones[ 8 ]; 

extern bool isRunning;  /// This is very important:  When true, the audio capture
                        /// loop (this is an event-blocking thread) will continue
                        /// run.  When false, the loop will exit and the thread
                        /// will end.

extern HANDLE gAudioSamplesReadyEvent;

#define SIZE_OF_QUEUE (800)  /* 100ms of data @ 8000Hz sampling rate */

extern BYTE pcmQueue[ SIZE_OF_QUEUE ];

extern BOOL mvcInitModel();

extern void pcmEnqueue( BYTE data );


// TODO: Consider having the thread have an indicator light that its running


// TODO: I should convert this to a read-only interface
