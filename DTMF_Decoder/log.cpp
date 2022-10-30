///////////////////////////////////////////////////////////////////////////////
//          University of Hawaii, College of Engineering
//          DTMF_Decoder - EE 469 - Fall 2022
//
//  A Windows Desktop C program that decodes DTMF tones
//
/// Generic Windows logging utility
///
/// Requirements for our logger:
///   - Log to `OutputDebugString` with the flexibility of `printf`
///   - Both wide and narrow logging
///   - For log levels #LOG_LEVEL_FATAL, #LOG_LEVEL_ERROR and #LOG_LEVEL_WARN also show a Dialog Box
///   - Bounds checking on the string buffer
///   - Hold the buffer on the stack so it's both thread safe and re-entrant safe
/// 
/// @file    log.cpp
/// @version 1.0
///
/// @author  Mark Nelson <marknels@hawaii.edu>
/// @date    29_Oct_2022
///////////////////////////////////////////////////////////////////////////////

#include "framework.h"    // Standard system include files
#include "log.h"          // For yourself

#include <stdio.h>
#include <stdarg.h>

/// Save a random number at the end of the string buffer.  When we're done, 
/// make sure it's untouched.
#define STACK_GUARD 0xed539d63

/// Log at the the INFO level
#define LOG_INFO( format, ... ) log( LOG_LEVEL_TRACE, APP_NAME, __FUNCTION__, format, __VA_ARGS__ ); 


/// Generic logging function
/// @param logLevel  The level of this logging event
/// @param appName   The name of the application
/// @param functionName  The name of the function
/// @param format        The formatr 
void log( const logLevels_t logLevel, const char* appName, const char* functionName, const char* format, ... ) {
   /// Silently `return` if appName, functionName or format is `NULL`
   if ( appName == NULL || functionName == NULL || format == NULL )
      return;

   struct buffer_t {
      CHAR  sBuf[ MAX_LOG_STRING ];
      DWORD dwGuard;
   } buffer;

   buffer.dwGuard = STACK_GUARD;

   va_list     args;
   int         numCharsWritten = 0;
   int         bufCharsRemaining = MAX_LOG_STRING;

   va_start( args, format );
   // va_start does not return a result code

   bufCharsRemaining -= sprintf_s( buffer.sBuf, bufCharsRemaining, "%s: ", functionName );

   /// Under normal circumstances, `vsprintf_s` will fail on a buffer overflow
   /// (and it takes `\0` into account).
   bufCharsRemaining -= vsprintf_s( buffer.sBuf + numCharsWritten, bufCharsRemaining, format, args );

   // Commented out below because I trust vsprintf_s
   // buffer.sBuf[ bufCharsRemaining - 1 ] = '\0';  // Null terminate the buffer (just to be sure)

   /// If we have overrun the buffer or violated the stack guard, then I'd like
   /// to fail fast and immediately terminate.
   if ( bufCharsRemaining < 0 || buffer.dwGuard != STACK_GUARD ) {
      OutputDebugStringA( "VIOLATED STACK GUARD in Logger.  Exiting immediately." );
      _ASSERT_EXPR( FALSE, L"VIOLATED STACK GUARD in Logger.  Exiting immediately." );
      /// @todo sweep for new @see references and format nicely
   }

   OutputDebugStringA( buffer.sBuf );
}


/// Test the logging functionality
void logTest() {
   LOG_INFO( "Testing LOG_INFO" );
   LOG_INFO( "789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456" );
   LOG_INFO( "7890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567" );
   
   // the following lines will fail an assertion check in vsprintf_s
   // LOG_INFO( "78901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678" );
   // LOG_INFO( "789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789" );
}



//sprintf_s( sBuf, sizeof( sBuf ), __FUNCTION__ ":  Start Goertzel DFT thread index=%zu", index );
//OutputDebugStringA( sBuf );

//OutputDebugStringA( APP_NAME ": Starting" );

//swprintf_s( wsBuf, sizeof( wsBuf ), __FUNCTIONW__ L":  Device ID=%s", spwstrDeviceId );
//OutputDebugStringW( wsBuf );

