///////////////////////////////////////////////////////////////////////////////
//          University of Hawaii, College of Engineering
//          DTMF_Decoder - EE 469 - Fall 2022
//
/// A Windows Desktop C program that decodes DTMF tones
///
/// @file    DTMF_Decoder.h
/// @version 1.0
///
/// @author  Mark Nelson <marknels@hawaii.edu>
/// @date    10_Oct_2022
///////////////////////////////////////////////////////////////////////////////

#pragma once


/// The name of the application (narrow character)
#define APP_NAME     "DTMF Decoder"

/// The name of the application (wide character)
#define APP_NAME_W  L"DTMF Decoder"


extern void gracefulShutdown();
