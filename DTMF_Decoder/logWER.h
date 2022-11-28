///////////////////////////////////////////////////////////////////////////////
//          University of Hawaii, College of Engineering
//          DTMF_Decoder - EE 469 - Fall 2022
//
//  A Windows Desktop C program that decodes DTMF tones
//
/// Generic Windows Error Reporting utility
///
/// @file    logWER.h
/// @author  Mark Nelson <marknels@hawaii.edu>
///////////////////////////////////////////////////////////////////////////////

#pragma once


/// C preprocesor trick that converts `define`d constants into strings
#define RESOURCE_ID_TO_STR( resourceId) #resourceId


extern BOOL logWerInit();
extern BOOL logWerEvent(
   _In_   const logLevels_t logLevel,
   _In_z_ const WCHAR*      resourceName,
   _In_   const UINT        resourceId,
   _In_z_ const WCHAR*      logMsg
);
extern BOOL logWerSubmit();
extern BOOL logWerCleanup();
