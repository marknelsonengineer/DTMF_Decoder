///////////////////////////////////////////////////////////////////////////////
//          University of Hawaii, College of Engineering
//          DTMF_Decoder - EE 469 - Fall 2022
//
//  A Windows Desktop C program that decodes DTMF tones
//
/// Generic Windows logging utility
///
/// Requirements for the logger:
///   - Log to `OutputDebugString` with the flexibility of `printf`
///   - Both wide and narrow logging
///   - For log levels #LOG_LEVEL_FATAL, #LOG_LEVEL_ERROR and #LOG_LEVEL_WARN 
///     also show a Message Box
///   - Bounds checking on the string buffer
///   - Hold the buffer on the stack so it's both thread safe and re-entrant safe
/// 
/// @todo sweep for new @see references and format nicely
///
/// @file    log.cpp
/// @version 1.0
///
/// @author  Mark Nelson <marknels@hawaii.edu>
/// @date    29_Oct_2022
///////////////////////////////////////////////////////////////////////////////

#include "framework.h"    // Standard system include files
#include "log.h"          // For yourself

#include <stdio.h>        // For sprintf_s
#include <stdarg.h>       // For va_start


/// Save a random number at the end of the string buffer.  When we're done, 
/// make sure it's untouched.
/// 
/// If we overrun the buffer and violate the stack guard, then fail fast by
/// throwing an `_ASSERT`.
#define STACK_GUARD 0xed539d63

/// Log at the the #LOG_LEVEL_TRACE level
#define LOG_TRACE( format, ... ) log( LOG_LEVEL_TRACE, APP_NAME, __FUNCTION__, format, __VA_ARGS__ ); 

/// Log at the the #LOG_LEVEL_DEBUG level
#define LOG_DEBUG( format, ... ) log( LOG_LEVEL_DEBUG, APP_NAME, __FUNCTION__, format, __VA_ARGS__ ); 

/// Log at the the #LOG_LEVEL_INFO level
#define LOG_INFO( format, ... )  log( LOG_LEVEL_INFO,  APP_NAME, __FUNCTION__, format, __VA_ARGS__ ); 

/// Log at the the #LOG_LEVEL_WARN level
#define LOG_WARN( format, ... )  log( LOG_LEVEL_WARN,  APP_NAME, __FUNCTION__, format, __VA_ARGS__ ); 

/// Log at the the #LOG_LEVEL_ERROR level
#define LOG_ERROR( format, ... ) log( LOG_LEVEL_ERROR, APP_NAME, __FUNCTION__, format, __VA_ARGS__ ); 

/// Log at the the #LOG_LEVEL_FATAL level
#define LOG_FATAL( format, ... ) log( LOG_LEVEL_FATAL, APP_NAME, __FUNCTION__, format, __VA_ARGS__ ); 



/// Generic logging function
/// 
/// This is intended to be called through the logging macros like #LOG_INFO.
/// It is not intended to be called directly.
/// 
/// I'm choosing to make this a function rather than an inline.  Header-only 
/// files are great, but I'd like to keep the buffer and all of the `vsprintf` 
/// stuff in a self-contained library rather than push it into the caller's source.
/// 
/// @param logLevel      The level of this logging event
/// @param appName       The name of the application
/// @param functionName  The name of the function
/// @param format, ...   `printf`-style formatting 
void log( 
   _In_ const logLevels_t logLevel, 
   _In_ const char*       appName, 
   _In_ const char*       functionName, 
   _In_ const char*       format, 
   _In_ ... ) {
   /// Silently `return` if `appName`, `functionName` or `format` is `NULL`
   if ( appName == NULL || functionName == NULL || format == NULL )
      return;

   struct buffer_t {
      CHAR  sBuf[ MAX_LOG_STRING ];
      DWORD dwGuard;
   } buffer;

   buffer.dwGuard = STACK_GUARD;

   va_list args;
   int     numCharsWritten   = 0;
   int     bufCharsRemaining = MAX_LOG_STRING;

   va_start( args, format );
   // va_start does not return a result code

   numCharsWritten = sprintf_s( buffer.sBuf, bufCharsRemaining, "%s: ", functionName );
   // sprintf_s returns the number of characters written

   bufCharsRemaining -= numCharsWritten;

   /// Under normal circumstances, `vsprintf_s` will throw an `_ASSERT` failure
   /// on a buffer overflow (and it takes `\0` into account).
   bufCharsRemaining -= vsprintf_s( buffer.sBuf + numCharsWritten, bufCharsRemaining, format, args );
   // vsprintf_s returns the number of characters written

   // Commented out below because I trust vsprintf_s
   // buffer.sBuf[ bufCharsRemaining - 1 ] = '\0';  // Null terminate the buffer (just to be sure)

/// If we overrun the buffer and violate the stack guard, then fail fast by
/// throwing an `_ASSERT`.
   if ( bufCharsRemaining < 0 || buffer.dwGuard != STACK_GUARD ) {
      OutputDebugStringA( "VIOLATED STACK GUARD in Logger.  Exiting immediately." );
      _ASSERT_EXPR( FALSE, L"VIOLATED STACK GUARD in Logger.  Exiting immediately." );
   }

   OutputDebugStringA( buffer.sBuf );

   if ( logLevel == LOG_LEVEL_WARN ) {
      MessageBoxA( NULL, buffer.sBuf, appName, MB_OK | MB_ICONWARNING );
   } else if ( logLevel == LOG_LEVEL_ERROR ) {
      MessageBoxA( NULL, buffer.sBuf, appName, MB_OK | MB_ICONERROR );
   } else if ( logLevel == LOG_LEVEL_FATAL ) {
      MessageBoxA( NULL, buffer.sBuf, appName, MB_OK | MB_ICONSTOP );
   }
}


/// Test the logging functionality
///
/// This is not normally used, except for testing.
void logTest() {
   LOG_TRACE( "Testing LOG_TRACE" );
   LOG_DEBUG( "Testing LOG_DEBUG" );
   LOG_INFO(  "Testing LOG_INFO"  );
   LOG_WARN(  "Testing LOG_WARN"  );
   LOG_ERROR( "Testing LOG_ERROR" );
   LOG_FATAL( "Testing LOG_FATAL" );

   // Bounds testing... the following two lines should succeed
   LOG_INFO( "012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456" );
   LOG_INFO( "0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567" );
   

   //The following excessively long lines will fail an assertion check in vsprintf_s
   // This takes into account that "logTest: " is 10 characters long
   // LOG_INFO( "01234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678" );
   // LOG_INFO( "012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789" );
}



//sprintf_s( sBuf, sizeof( sBuf ), __FUNCTION__ ":  Start Goertzel DFT thread index=%zu", index );
//OutputDebugStringA( sBuf );

//OutputDebugStringA( APP_NAME ": Starting" );

//swprintf_s( wsBuf, sizeof( wsBuf ), __FUNCTIONW__ L":  Device ID=%s", spwstrDeviceId );
//OutputDebugStringW( wsBuf );

