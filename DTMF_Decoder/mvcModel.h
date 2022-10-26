///////////////////////////////////////////////////////////////////////////////
//          University of Hawaii, College of Engineering
//          DTMF_Decoder - EE 469 - Fall 2022
//
//  A Windows Desktop C program that decodes DTMF tones
//
/// The model will hold state between the controller and view code
/// 
/// @file mvcModel.h
/// @version 1.0
///
/// @author Mark Nelson <marknels@hawaii.edu>
/// @date   10_Oct_2022
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <windows.h>  // For WCHAR, BYTE, etc.
#include "audio.h"    // For queueSize

#define NUMBER_OF_DTMF_TONES (8)

/// Hold display information (detected & label) as well as pre-computed information
/// (sine, cosine, coeff) for the Goertzel Magnitude calculation for each 
/// individual DTMF tone
typedef struct {
   int   index;              ///< The index of the tone in the dtmfTones array
   float frequency;          ///< The DTMF tone's frequency
   bool  detected;           ///< `true` if the tone is present, `false` if it's not
   WCHAR label[ 16 ];        ///< A Wide-char label for the tone
   float goertzelMagnitude;  ///< The latest magnitude
   float sine;               ///< Pre-computed sin offset for the frequency and sample rate
   float cosine;             ///< Pre-computed cosine offset for the frequency and smaple rate
   float coeff;              ///< Pre-computed Goertzel coefficient
} dtmfTones_t;

/// @todo I should convert this to a read-only interface
extern dtmfTones_t dtmfTones[ NUMBER_OF_DTMF_TONES ];

/// `true` if the Goertzel detects that any DTMF tone has changed.
/// `false` if there are no recent changes (so there's no need to repaint the screen).
extern bool hasDtmfTonesChanged;

extern void editToneDetectedStatus( size_t toneIndex, bool detectedStatus );

extern bool isRunning;  /// This is very important:  When true, the audio capture
                        /// loop (this is an event-blocking thread) will continue
                        /// run.  When false, the loop will exit and the thread
                        /// will end.

extern HANDLE gAudioSamplesReadyEvent;  /// This event is signaled when the audio
                                        /// driver has some data to send.  It's
                                        /// what makes this program event-driven.

#define SIZE_OF_QUEUE_IN_MS (150)

extern BOOL mvcInitModel();   /// Initialize the model

extern "C" BYTE * pcmQueue;  ///< We need to make this visible so we can the queue from Goertzel
extern "C" size_t queueHead;  ///< We need to make this visible so we can read the queue from Goertzel
extern BOOL pcmSetQueueSize( size_t size );  /// Set the size of the queue

/// Enqueue a byte of PCM data to `pcmQueue`
inline void pcmEnqueue( BYTE data ) {
   //_ASSERTE( pcmQueue != NULL );   /// @todo Convert all assert to _ASSERTE
   //assert( queueHead < queueSize );

   pcmQueue[ queueHead++ ] = data ;

   if ( queueHead >= queueSize ) {  // This is an efficient wraparound
      queueHead = 0;
   }

   // _ASSERTE( _CrtCheckMemory() );
}

extern void pcmReleaseQueue();         /// Release the memory allocated for the queue

/// @todo Consider having the thread have an indicator light that its running
