///////////////////////////////////////////////////////////////////////////////
//          University of Hawaii, College of Engineering
//          DTMF_Decoder - EE 469 - Fall 2022
//
//  A Windows Desktop C program that decodes DTMF tones
//
/// Generic Windows logging utility
///
/// @file    log.h
/// @version 1.0
///
/// @author  Mark Nelson <marknels@hawaii.edu>
/// @date    29_Oct_2022
///////////////////////////////////////////////////////////////////////////////

#pragma once

/// Logging sybsystem log levels
enum logLevels_t {
   LOG_LEVEL_TRACE = 0,  ///< Log at TRACE level
   LOG_LEVEL_DEBUG,      ///< Log at DEBUG level
   LOG_LEVEL_INFO,       ///< Log at INFO level
   LOG_LEVEL_WARN,       ///< Log at WARN level (and display a DialogBox)
   LOG_LEVEL_ERROR,      ///< Log at ERROR level (and display a DialogBox)
   LOG_LEVEL_FATAL       ///< Log at FATAL level (and display a DialogBox)
};


/// The maximum length of a log entry.  Anything longer will be truncated.
#define MAX_LOG_STRING 128


extern void logTest();
