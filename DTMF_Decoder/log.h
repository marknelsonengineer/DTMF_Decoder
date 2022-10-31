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
/// Logs will have a `\n` appended to them (for the Windows debugger)
#define MAX_LOG_STRING 128


/// Log at the the #LOG_LEVEL_TRACE level
#define LOG_TRACE( format, ... ) logA( LOG_LEVEL_TRACE, APP_NAME, __FUNCTION__, format, __VA_ARGS__ ); 

/// Log at the the #LOG_LEVEL_DEBUG level
#define LOG_DEBUG( format, ... ) logA( LOG_LEVEL_DEBUG, APP_NAME, __FUNCTION__, format, __VA_ARGS__ ); 

/// Log at the the #LOG_LEVEL_INFO level
#define LOG_INFO( format, ... )  logA( LOG_LEVEL_INFO,  APP_NAME, __FUNCTION__, format, __VA_ARGS__ ); 

/// Log at the the #LOG_LEVEL_WARN level
#define LOG_WARN( format, ... )  logA( LOG_LEVEL_WARN,  APP_NAME, __FUNCTION__, format, __VA_ARGS__ ); 

/// Log at the the #LOG_LEVEL_ERROR level
#define LOG_ERROR( format, ... ) logA( LOG_LEVEL_ERROR, APP_NAME, __FUNCTION__, format, __VA_ARGS__ ); 

/// Log at the the #LOG_LEVEL_FATAL level
#define LOG_FATAL( format, ... ) logA( LOG_LEVEL_FATAL, APP_NAME, __FUNCTION__, format, __VA_ARGS__ ); 


/// Log at the the #LOG_LEVEL_TRACE level
#define LOG_TRACE_W( format, ... ) logW( LOG_LEVEL_TRACE, APP_NAME_W, __FUNCTIONW__, format, __VA_ARGS__ ); 

/// Log at the the #LOG_LEVEL_DEBUG level
#define LOG_DEBUG_W( format, ... ) logW( LOG_LEVEL_DEBUG, APP_NAME_W, __FUNCTIONW__, format, __VA_ARGS__ ); 

/// Log at the the #LOG_LEVEL_INFO level
#define LOG_INFO_W( format, ... )  logW( LOG_LEVEL_INFO,  APP_NAME_W, __FUNCTIONW__, format, __VA_ARGS__ ); 

/// Log at the the #LOG_LEVEL_WARN level
#define LOG_WARN_W( format, ... )  logW( LOG_LEVEL_WARN,  APP_NAME_W, __FUNCTIONW__, format, __VA_ARGS__ ); 

/// Log at the the #LOG_LEVEL_ERROR level
#define LOG_ERROR_W( format, ... ) logW( LOG_LEVEL_ERROR, APP_NAME_W, __FUNCTIONW__, format, __VA_ARGS__ ); 

/// Log at the the #LOG_LEVEL_FATAL level
#define LOG_FATAL_W( format, ... ) logW( LOG_LEVEL_FATAL, APP_NAME_W, __FUNCTIONW__, format, __VA_ARGS__ ); 


extern void logTest();
