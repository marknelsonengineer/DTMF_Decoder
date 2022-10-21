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

#define NUMBER_OF_DTMF_TONES (8)

typedef struct {
   float frequency;    /// The DTMF tone
   bool  detected;     /// True if the tone is present, false if it's not
   WCHAR label[ 16 ];  /// A label for the tone
} dtmfTones_t;

extern dtmfTones_t dtmfTones[ NUMBER_OF_DTMF_TONES ];

extern bool hasDtmfTonesChanged;

extern void editToneDetectedStatus( size_t toneIndex, bool detectedStatus );

extern bool isRunning;  /// This is very important:  When true, the audio capture
                        /// loop (this is an event-blocking thread) will continue
                        /// run.  When false, the loop will exit and the thread
                        /// will end.

extern HANDLE gAudioSamplesReadyEvent;  /// This event is signaled when the audio
                                        /// driver has some data to send.  It's
                                        /// what makes this program event-driven.

#define SIZE_OF_QUEUE (800)  /* 100ms of data @ 8000Hz sampling rate */

extern BYTE pcmQueue[ SIZE_OF_QUEUE ];  /// A circular queue (really, a ring buffer)
                                        /// of PCM samples to analyze

extern BOOL mvcInitModel();   /// Initialize the model

extern void pcmEnqueue( BYTE data );   /// Enqueue a byte of PCM data to `pcmQueue`


// TODO: Consider having the thread have an indicator light that its running


// TODO: I should convert this to a read-only interface
