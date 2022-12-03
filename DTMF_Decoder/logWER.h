///////////////////////////////////////////////////////////////////////////////
//          University of Hawaii, College of Engineering
//          DTMF_Decoder - EE 469 - Fall 2022
//
//  A Windows Desktop C program that decodes DTMF tones
//
/// Extend log.cpp to use Windows Error Reporting utility
///
/// @file    logWER.h
/// @author  Mark Nelson <marknels@hawaii.edu>
///////////////////////////////////////////////////////////////////////////////

#pragma once

extern RETURN_BOOL logWerInit();

extern RETURN_BOOL logWerEvent(
   _In_       const logLevels_t logLevel,
   _In_opt_z_ const PCWSTR      resourceName,
   _In_       const UINT        resourceId,
   _In_z_     const PCWSTR      logMsg
);
extern RETURN_BOOL logWerSubmit();
extern RETURN_BOOL logWerCleanup();
