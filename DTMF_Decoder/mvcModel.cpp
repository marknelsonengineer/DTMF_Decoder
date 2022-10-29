///////////////////////////////////////////////////////////////////////////////
//          University of Hawaii, College of Engineering
//          DTMF_Decoder - EE 469 - Fall 2022
//
//  A Windows Desktop C program that decodes DTMF tones
//
/// The model holds the state between the various modules
///
/// @file mvcModel.cpp
/// @version 1.0
///
/// @see https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/malloc-dbg                
/// @see https://learn.microsoft.com/en-us/previous-versions/windows/desktop/legacy/aa366877(v=vs.85)
/// @see https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/free-dbg                  
/// @see https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/crtcheckmemory            
///
/// @author Mark Nelson <marknels@hawaii.edu>
/// @date   10_Oct_2022
///////////////////////////////////////////////////////////////////////////////

#include "framework.h"    // Standard system include files
#include <malloc.h>       // For malloc and free

#include "mvcModel.h"     // For yo bad self


/// Currently does nothing, but it's good to have around
BOOL mvcModelInit() {
   return TRUE;
}


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


bool isRunning = false;


HWND ghMainWindow = NULL;


/// @cond Doxygen_Suppress
BYTE* pcmQueue = NULL;
size_t queueHead = 0;
size_t queueSize = 0;
/// @endcond


BOOL pcmSetQueueSize( _In_ size_t size ) {

   _ASSERTE( pcmQueue == NULL );
   _ASSERTE( queueSize == 0 );
   _ASSERTE( size != 0 );

   pcmQueue = (BYTE*)_malloc_dbg(size, _CLIENT_BLOCK, __FILE__, __LINE__);
   if ( pcmQueue == NULL ) {
      OutputDebugStringA( __FUNCTION__ ":  Failed to allocate memory for PCM queue" );
      return FALSE;
   }

   queueSize = size;
   queueHead = 0;

   SecureZeroMemory( pcmQueue, queueSize );

   _ASSERTE( pcmQueue != NULL );
   _ASSERTE( queueSize != 0 );

   return TRUE;
}


void pcmReleaseQueue() {
   if ( pcmQueue == NULL )
      return;

   _ASSERTE( _CrtCheckMemory() );  // Check memory and see if there's a problem

   _free_dbg( pcmQueue, _CLIENT_BLOCK );
}


DWORD   mmcssTaskIndex          = 0;
HANDLE  gAudioSamplesReadyEvent = NULL;
