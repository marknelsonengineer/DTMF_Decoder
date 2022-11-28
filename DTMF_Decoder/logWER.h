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

extern BOOL logWerInit( 
   _In_         HINSTANCE* phInst,
   _In_         HWND*      phWindow,
   _In_z_ const CHAR*      pAppName,
   _In_z_ const WCHAR*     pwAppName
);

extern BOOL logWerEvent();
extern BOOL logWerSubmit();
extern BOOL logWerCleanup();
