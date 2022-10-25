///////////////////////////////////////////////////////////////////////////////
//          University of Hawaii, College of Engineering
//          DTMF_Decoder - EE 469 - Fall 2022
//
/// A Windows Desktop C program that decodes DTMF tones
///
/// The model will hold state between the various modules.
/// 
/// TODO:  Consolidate all exported global variables into here
///
/// @file mvcModel.cpp
/// @version 1.0
///
/// @author Mark Nelson <marknels@hawaii.edu>
/// @date   10_Oct_2022
///////////////////////////////////////////////////////////////////////////////

#include "framework.h"    // Standard system include files
#include <assert.h>       // For assert()
#include <crtdbg.h>       // For _malloc_dbg()
#include "mvcModel.h"     // For yo bad self

/// @note:  There are several other members to dtmfTones that are computed and
///         set by goertzel_init()
dtmfTones_t dtmfTones[ NUMBER_OF_DTMF_TONES ] = {
   { 0,  697.0, false, L"697" },
   { 1,  770.0, false, L"770" },  
   { 2,  852.0, false, L"852" },
   { 3,  941.0, false, L"941" },
   { 4, 1209.0, false, L"1209" },
   { 5, 1336.0, false, L"1336" }, 
   { 6, 1477.0, false, L"1477" }, 
   { 7, 1633.0, false, L"1633" }
};


bool hasDtmfTonesChanged = false;


void editToneDetectedStatus( size_t toneIndex, bool detectedStatus ) {
   assert( toneIndex < NUMBER_OF_DTMF_TONES );

   if ( dtmfTones[ toneIndex ].detected != detectedStatus ) {
      dtmfTones[ toneIndex ].detected = detectedStatus;
      hasDtmfTonesChanged = true;
   }
}


bool isRunning = false;


static size_t queueSize = 0;
BYTE* pcmQueue = NULL;


BOOL pcmSetQueueSize( size_t size ) {
   //assert( pcmQueue == NULL );
   //assert( queueSize == 0 );

   pcmQueue = (BYTE*)_malloc_dbg(size, _CLIENT_BLOCK, __FILE__, __LINE__);
   if ( pcmQueue == NULL ) {
      OutputDebugStringA( __FUNCTION__ ":  Failed to allocate memory for PCM queue" );
      return FALSE;
   }

   queueSize = size;
   queueHead = 0;

   ZeroMemory( pcmQueue, queueSize );

   assert( pcmQueue != NULL );
   assert( queueSize != 0 );

   return TRUE;
}


void pcmReleaseQueue() {
   if ( pcmQueue == NULL )
      return;

   _ASSERTE( _CrtCheckMemory() );  // Check memory and see if there's a problem

   _free_dbg( pcmQueue, _CLIENT_BLOCK );
}


/// Currently does nothing, but it's good to have around
BOOL mvcInitModel() {
   return TRUE;
}


size_t queueHead = 0;  /// Points to the next available byte for writing
                       /// This is thread safe because all of the threads
                       /// read from the same, unchanging queue.
                       /// 
                       /// It's the readHead pointer that needs to be thread
                       /// local.