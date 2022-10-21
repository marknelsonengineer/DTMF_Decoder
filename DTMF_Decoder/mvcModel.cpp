///////////////////////////////////////////////////////////////////////////////
//          University of Hawaii, College of Engineering
//          DTMF_Decoder - EE 469 - Fall 2022
//
/// A Windows Desktop C program that decodes DTMF tones
///
/// The model will hold state between the controller and view code
///
/// @file mvcModel.cpp
/// @version 1.0
///
/// @author Mark Nelson <marknels@hawaii.edu>
/// @date   10_Oct_2022
///////////////////////////////////////////////////////////////////////////////

#include "framework.h"    // Standard system include files
#include <assert.h>       // For assert()

#include "mvcModel.h"

dtmfTones_t dtmfTones[ 8 ] = {
   { 697.0, false, L"697" },
   { 770.0, false, L"770" },  
   { 852.0, false, L"852" },
   { 941.0, false, L"941" },
   {1209.0, false, L"1209" },
   {1336.0, false, L"1336" }, 
   {1477.0, false, L"1477" }, 
   {1633.0, false, L"1633" }
};

extern bool isRunning = false;

BYTE pcmQueue[SIZE_OF_QUEUE];

BOOL mvcInitModel() {
   ZeroMemory( pcmQueue, SIZE_OF_QUEUE );

   return TRUE;
}

static INT64 queueHead = 0;  // Points to the next available byte

void pcmEnqueue( BYTE data ) {
   assert( queueHead < sizeof( pcmQueue ) );
   pcmQueue[ queueHead++ ] ;

   queueHead %= sizeof( pcmQueue );  // There are more clever/efficient ways to do this, but this is very clear
}
