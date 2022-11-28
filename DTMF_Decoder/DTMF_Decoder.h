///////////////////////////////////////////////////////////////////////////////
//          University of Hawaii, College of Engineering
//          DTMF_Decoder - EE 469 - Fall 2022
//
/// A Windows Desktop C program that decodes DTMF tones
///
/// @file    DTMF_Decoder.h
/// @author  Mark Nelson <marknels@hawaii.edu>
///////////////////////////////////////////////////////////////////////////////

#pragma once

/// The `const` name of the application (narrow character) - does not change
/// with localization
#define APP_NAME     "DTMF Decoder"

/// The `const` name of the application (wide character) - does not change
/// with localization
#define APP_NAME_W  L"DTMF Decoder"

/// The application's return code if it ends normally
#define EXIT_SUCCESS 0

/// The application's return code if it ends abnormally
#define EXIT_FAILURE 1

extern void gracefulShutdown();
