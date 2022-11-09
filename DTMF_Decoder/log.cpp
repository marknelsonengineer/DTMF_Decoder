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
///   - Append a \n because that's how the Windws debugger likes to print
///     output
///
/// ## APIs Used
/// | API                  | Link                                                                                                                              |
/// |--------------------- | ----------------------------------------------------------------------------------------------------------------------------------|
/// | `va_arg`             | https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/va-arg-va-copy-va-end-va-start?view=msvc-170                    |
/// | `sprintf_s`          | https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/sprintf-s-sprintf-s-l-swprintf-s-swprintf-s-l?view=msvc-170     |
/// | `vsprintf_s`         | https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/vsprintf-s-vsprintf-s-l-vswprintf-s-vswprintf-s-l?view=msvc-170 |
/// | `OutputDebugStringA` | https://learn.microsoft.com/en-us/windows/win32/api/debugapi/nf-debugapi-outputdebugstringa                                       |
/// | `_ASSERT_EXPR`       | https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/assert-asserte-assert-expr-macros?view=msvc-170                 |
/// | `MessageBoxA`        | https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-messageboxa                                                |
/// | `swprintf_s`         | https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/sprintf-s-sprintf-s-l-swprintf-s-swprintf-s-l?view=msvc-170     |
/// | `vswprintf_s`        | https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/vsprintf-s-vsprintf-s-l-vswprintf-s-vswprintf-s-l?view=msvc-170 |
/// | `OutputDebugStringW` | https://learn.microsoft.com/en-us/windows/win32/api/debugapi/nf-debugapi-outputdebugstringw                                       |
/// | `MessageBoxW`        | https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-messageboxw                                                |
/// | `MessageBeep`        | https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-messagebeep                                                |
///
/// @file    log.cpp
/// @author  Mark Nelson <marknels@hawaii.edu>
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


/// The handle to a window that will "own" the message box popups.
static HWND shMainWindow = NULL;


/// Initialize the logger
///
/// Per Raymond Chen's book, all windows, including message boxes, should be owned by their
/// calling window.  I don't want to pass an hWnd into each log (although, in the future
/// that may be necessary).  So, I'll initialize the logger and set #shMainWindow.
///
/// @param hWindow The window that will own the log message boxes (usually the
///                application's main window).
///
/// @return `true` if successful.  `false` if there was a problem.
BOOL logInit( _In_ const HWND hWindow ) {
   shMainWindow = hWindow;

   return TRUE;
}


/// Cleanup the logger
///
/// @return `true` if successful.  `false` if there was a problem.
BOOL logCleanup() {
   shMainWindow = NULL;  /// Set #shMainWindow to `NULL`

   return TRUE;
}


/// Generic logging function (narrow character)
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
void logA(
   _In_ const logLevels_t logLevel,
   _In_ const CHAR*       appName,
   _In_ const CHAR*       functionName,
   _In_ const CHAR*       format,
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
   CHAR*   pBufferHead       = buffer.sBuf;

   va_start( args, format );
   // va_start does not return a result code

   numCharsWritten = sprintf_s( pBufferHead, bufCharsRemaining, "%s: ", functionName );
   // sprintf_s returns the number of characters written
   bufCharsRemaining -= numCharsWritten;
   pBufferHead       += numCharsWritten;

   /// Under normal circumstances, `vsprintf_s` will throw an `_ASSERT` failure
   /// on a buffer overflow (and it takes `\0` into account).
   numCharsWritten = vsprintf_s( pBufferHead, bufCharsRemaining, format, args );
   bufCharsRemaining -= numCharsWritten;
   pBufferHead       += numCharsWritten;

   /// Append a `\n` for MSVC's debugger
   numCharsWritten = sprintf_s( pBufferHead, bufCharsRemaining, "\n" );
   bufCharsRemaining -= numCharsWritten;
   pBufferHead       += numCharsWritten;

   (void) pBufferHead;  // Suppress a compiler warning that pBufferHead
                     // is not checked after this.  No code is generated.

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
      MessageBoxA( shMainWindow, buffer.sBuf, appName, MB_OK | MB_ICONWARNING );
   } else if ( logLevel == LOG_LEVEL_ERROR ) {
      MessageBeep( MB_ICONERROR );   // No need to check for a result code
      MessageBoxA( shMainWindow, buffer.sBuf, appName, MB_OK | MB_ICONERROR );
   } else if ( logLevel == LOG_LEVEL_FATAL ) {
      MessageBeep( MB_ICONSTOP );    // No need to check for a result code
      MessageBoxA( shMainWindow, buffer.sBuf, appName, MB_OK | MB_ICONSTOP );
   }
}


/// Generic logging function (wide character)
///
/// This is intended to be called through the logging macros like #LOG_INFO_W.
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
void logW(
   _In_ const logLevels_t logLevel,
   _In_ const WCHAR* appName,
   _In_ const WCHAR* functionName,
   _In_ const WCHAR* format,
   _In_ ... ) {
   /// Silently `return` if `appName`, `functionName` or `format` is `NULL`
   if ( appName == NULL || functionName == NULL || format == NULL )
      return;

   struct buffer_t {
      WCHAR sBuf[ MAX_LOG_STRING ];
      DWORD dwGuard;
   } buffer;

   buffer.dwGuard = STACK_GUARD;

   va_list args;
   int     numCharsWritten   = 0;
   int     bufCharsRemaining = MAX_LOG_STRING;
   WCHAR*  pBufferHead       = buffer.sBuf;

   va_start( args, format );
   // va_start does not return a result code

   numCharsWritten = swprintf_s( pBufferHead, bufCharsRemaining, L"%s: ", functionName );
   // swprintf_s returns the number of characters written
   bufCharsRemaining -= numCharsWritten;
   pBufferHead       += numCharsWritten;

   /// Under normal circumstances, `vswprintf_s` will throw an `_ASSERT` failure
   /// on a buffer overflow (and it takes `\0` into account).
   numCharsWritten = vswprintf_s( pBufferHead, bufCharsRemaining, format, args );
   bufCharsRemaining -= numCharsWritten;
   pBufferHead       += numCharsWritten;

   /// Append a `\n` for MSVC's debugger
   numCharsWritten = swprintf_s( pBufferHead, bufCharsRemaining, L"\n" );
   bufCharsRemaining -= numCharsWritten;
   pBufferHead       += numCharsWritten;

   (void) pBufferHead;  // Suppress a compiler warning that pBufferHead
                        // is not checked after this.  No code is generated.

/// If we overrun the buffer and violate the stack guard, then fail fast by
/// throwing an `_ASSERT`.
   if ( bufCharsRemaining < 0 || buffer.dwGuard != STACK_GUARD ) {
      OutputDebugStringW( L"VIOLATED STACK GUARD in Logger.  Exiting immediately." );
      _ASSERT_EXPR( FALSE, L"VIOLATED STACK GUARD in Logger.  Exiting immediately." );
   }

   OutputDebugStringW( buffer.sBuf );

   if ( logLevel == LOG_LEVEL_WARN ) {
      MessageBoxW( shMainWindow, buffer.sBuf, appName, MB_OK | MB_ICONWARNING );
   } else if ( logLevel == LOG_LEVEL_ERROR ) {
      MessageBoxW( shMainWindow, buffer.sBuf, appName, MB_OK | MB_ICONERROR );
   } else if ( logLevel == LOG_LEVEL_FATAL ) {
      MessageBoxW( shMainWindow, buffer.sBuf, appName, MB_OK | MB_ICONSTOP );
   }
}


/// Test the logging functionality
///
/// This is not normally used, except for testing.
void logTest() {
   LOG_TRACE( "Testing LOG_TRACE (narrow)" );
   LOG_DEBUG( "Testing LOG_DEBUG (narrow)" );
   LOG_INFO(  "Testing LOG_INFO (narrow)"  );
   LOG_WARN(  "Testing LOG_WARN (narrow)"  );
   LOG_ERROR( "Testing LOG_ERROR (narrow)" );
   LOG_FATAL( "Testing LOG_FATAL (narrow)" );

   LOG_TRACE( "Testing LOG_TRACE (narrow) varargs [%d] [%s] [%f]", 1, "OK", 1.0 );

   LOG_TRACE_W( L"Testing LOG_TRACE (wide)" );
   LOG_DEBUG_W( L"Testing LOG_DEBUG (wide)" );
   LOG_INFO_W(  L"Testing LOG_INFO (wide)"  );
   LOG_WARN_W(  L"Testing LOG_WARN (wide)"  );
   LOG_ERROR_W( L"Testing LOG_ERROR (wide)" );
   LOG_FATAL_W( L"Testing LOG_FATAL (wide)" );

   LOG_TRACE_W( L"Testing LOG_TRACE (wide) varargs [%d] [%s] [%f]", 1, L"OK", 1.0 );

   // Bounds testing... the following lines should succeed
   LOG_INFO( "Narrow: 890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345" );
   LOG_INFO( "Narrow: 8901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456" );

   LOG_INFO_W( L"Wide: 67890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345" );
   LOG_INFO_W( L"Wide: 678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456" );

   // The following excessively long lines will fail an assertion check in vsprintf_s
   // This takes into account that "logTest: " is 10 characters long
// LOG_INFO( "Narrow: 89012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567" );

// LOG_INFO_W( L"Wide: 6789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567" );
}



// The following are the functions I am attempting to replace

//sprintf_s( sBuf, sizeof( sBuf ), __FUNCTION__ ":  Start Goertzel DFT thread index=%zu", index );
//OutputDebugStringA( sBuf );

//OutputDebugStringA( APP_NAME ": Starting" );

//swprintf_s( wsBuf, sizeof( wsBuf ), __FUNCTIONW__ L":  Device ID=%s", spwstrDeviceId );
//OutputDebugStringW( wsBuf );

