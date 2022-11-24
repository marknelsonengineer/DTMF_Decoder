///////////////////////////////////////////////////////////////////////////////
//          University of Hawaii, College of Engineering
//          DTMF_Decoder - EE 469 - Fall 2022
//
//  A Windows Desktop C program that decodes DTMF tones
//
/// The model holds the state between the various modules
///
/// ## APIs Used
/// | API                | Link                                                                                         |
/// |--------------------|----------------------------------------------------------------------------------------------|
/// | `_malloc_dbg`      | https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/malloc-dbg                 |
/// | `SecureZeroMemory` | https://learn.microsoft.com/en-us/previous-versions/windows/desktop/legacy/aa366877(v=vs.85) |
/// | `_free_dbg`        | https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/free-dbg                   |
/// | `_CrtCheckMemory`  | https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/crtcheckmemory             |
///
/// @file    mvcModel.cpp
/// @author  Mark Nelson <marknels@hawaii.edu>
///////////////////////////////////////////////////////////////////////////////

#include "framework.h"    // Standard system include files
#include <malloc.h>       // For malloc and free

#include "mvcModel.h"     // For yo bad self


/// Currently does nothing, but it's good to have around
///
/// @return `TRUE` if successful.  `FALSE` if there was a problem.
BOOL mvcModelInit() {
   return TRUE;
}


/// Cleans up any loose ends in the model
///
/// @return `TRUE` if successful.  `FALSE` if there was a problem.
BOOL mvcModelCleanup() {
   /// #gbIsRunning is always current and does not need cleaning

   /// Set #dtmfTones_t::detected to `false`
   for ( size_t i = 0 ; i < NUMBER_OF_DTMF_TONES ; i++ ) {
      gDtmfTones[ i ].detected = false;
   }

   /// Nothing to clean with #gdwMmcssTaskIndex

   /// We expect #audioCleanup to clean #ghAudioSamplesReadyEvent

   /// We expect DTMF_Decoder.cpp to clean #ghMainWindow

   /// #giApplicationReturnValue is always current and does not need cleaning

   /// Call #pcmReleaseQueue to clean #gPcmQueue, #gstQueueHead and #gstQueueSize
   pcmReleaseQueue();

   return TRUE;
}


dtmfTones_t gDtmfTones[ NUMBER_OF_DTMF_TONES ] = {
   { 0,  697.0, false, L"697" },   // Row 0
   { 1,  770.0, false, L"770" },   // Row 1
   { 2,  852.0, false, L"852" },   // Row 2
   { 3,  941.0, false, L"941" },   // Row 3
   { 4, 1209.0, false, L"1209" },  // Column 0
   { 5, 1336.0, false, L"1336" },  // Column 1
   { 6, 1477.0, false, L"1477" },  // Column 2
   { 7, 1633.0, false, L"1633" }   // Column 3
};


bool gbIsRunning = false;


HWND ghMainWindow = NULL;


int giApplicationReturnValue = EXIT_SUCCESS;  // Default to SUCCESS


/// @cond Doxygen_Suppress
BYTE*  gPcmQueue = NULL;
size_t gstQueueHead = 0;
size_t gstQueueSize = 0;
/// @endcond


BOOL pcmSetQueueSize( _In_ const size_t size ) {

   _ASSERTE( gPcmQueue == NULL );
   _ASSERTE( gstQueueSize == 0 );
   _ASSERTE( size != 0 );

   gPcmQueue = (BYTE*)_malloc_dbg(size, _CLIENT_BLOCK, __FILE__, __LINE__);
   if ( gPcmQueue == NULL ) {
      LOG_ERROR_R( IDS_MODEL_FAILED_TO_MALLOC );  // "Failed to allocate memory for PCM queue"
      return FALSE;
   }

   gstQueueSize = size;
   gstQueueHead = 0;

   SecureZeroMemory( gPcmQueue, gstQueueSize );

   _ASSERTE( gPcmQueue != NULL );
   _ASSERTE( gstQueueSize != 0 );
   _ASSERTE( gPcmQueue[ 0 ] == 0 );
   _ASSERTE( gPcmQueue[ gstQueueSize - 1 ] == 0 );

   return TRUE;
}


void pcmReleaseQueue() {
   if ( gPcmQueue != NULL ) {

      SecureZeroMemory( gPcmQueue, gstQueueSize );  // Zero out memory before releasing it

      _ASSERTE( _CrtCheckMemory() );  // Check memory and see if there's a problem

      _free_dbg( gPcmQueue, _CLIENT_BLOCK );

      gPcmQueue = NULL;
   }

   gstQueueHead = 0;
   gstQueueSize = 0;
}


DWORD   gdwMmcssTaskIndex        = 0;
HANDLE  ghAudioSamplesReadyEvent = NULL;
