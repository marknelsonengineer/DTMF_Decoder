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
///     also show a message box and beep
///   - Bounds checking on the string buffer
///   - Hold the buffer on the stack so it's both thread safe and re-entrant safe
///   - Append a `\n` because that's how the Windws debugger likes to print
///     output
///   - Help thread and mesage loop handlers register an error to be displayed
///     at a later time (this utilizes the Windows model of `GetLastError`
///     and `SetLastError`)
///
/// @see https://learn.microsoft.com/en-us/windows/win32/api/errhandlingapi/nf-errhandlingapi-getlasterror
/// @see https://learn.microsoft.com/en-us/windows/win32/api/errhandlingapi/nf-errhandlingapi-setlasterror
///
/// #### The Parent Window
/// A Windows logger is hard because
/// [MessageBox](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-messagebox)
/// [likes to have a handle to a parent window](https://devblogs.microsoft.com/oldnewthing/20050223-00/?p=36383).
/// Sometimes, an application doesn't have a parent window
/// because it either hasn't been created or it's been destroyed -- in which
/// case, the parent `hWnd` is `NULL`.
///
/// Ideally, I'd like a standalone Windows logger to find its own main window,
/// but [Win32 doesn't have an API for that](https://stackoverflow.com/questions/1119508/get-the-topmost-window-of-my-own-application-only-in-c).
/// Another approach is for every #LOG_TRACE, #LOG_DEBUG, ... macro to pass an
/// `hWnd` into it -- but this burdens the caller and I don't want to do that.
///
/// So, we have to set/initialize a Window when we start the logger and keep
/// it updated as the program evolves.  There's a couple of ways to do this:
///   1. Hold an `hWnd` in the logger and keep it up-to-date with a `setWindow()`
///   2. Hold a pointer to the application's **global** `hWnd` and expect the
///      application to keep it up-to-date.  DANGER:  The application's `hWnd`
///      must be a global.  It can't be a local or a parameter.
///
/// I'm choosing Option 2 because:
///   - It's simple and fire-and-forget.
///   - Most Win32 apps keep a global `hWnd`.
///   - It's always up-to-date:  When the global `hWnd` is set or cleared, so
///     is the logger's `hWnd`.
///
/// The risk is that someone (me) will use this logger sometime in the future
/// and initialize an `hWnd` pointer to a local or a parameter -- in which case
/// very bad things happen.  C'est la vie.
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


/// Pointer to an application's global windows handle.  This window will "own"
/// the message box popups.
static HWND* sphMainWindow = NULL;


/// Initialize the logger
///
/// Note: The logger **can** be called before it's initialized.  This is by
///       design.  The message box windows just won't have a parent window.
///
/// [Per Raymond Chen's book](https://devblogs.microsoft.com/oldnewthing/20050223-00/?p=36383),
/// all windows, including message boxes, should be
/// owned by their calling window.  I don't want to pass an `hWnd` into each log
/// (although, in the future that may be necessary).  So, I'll initialize the
/// logger and set #sphMainWindow to point to the app's windows handle, which
/// I expect the app will keep up-to-date for me.
///
/// @param phWindow The window that will own the log message boxes (usually the
///                 application's main window).  **This must be a global variable.**
///                 It is OK to set this to `NULL`
///
/// @return `TRUE` if successful.  `FALSE` if there was a problem.
BOOL logInit( _In_ HWND* phWindow ) {
   if ( phWindow != NULL ) {     // Test to ensure that phWindow points to
      HWND hWinTemp = *phWindow; // a valid address.  If it's invalid, this
      (void) hWinTemp;           // will segfault.
      _ASSERTE( TRUE );
   }

   sphMainWindow = phWindow;

   /// Call #logResetMsg to reset the last message logger
   logResetMsg();

   return TRUE;
}


/// Cleanup the logger
///
/// @return `TRUE` if successful.  `FALSE` if there was a problem.
BOOL logCleanup() {
   sphMainWindow = NULL;  /// Set #sphMainWindow to `NULL`

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
/// Note: The logger **can** be called before it's initialized.  This is by
///       design.  The message box windows just won't have a parent window.
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
      MessageBoxA( ( ( sphMainWindow == NULL ) ? NULL : *sphMainWindow ), buffer.sBuf, appName, MB_OK | MB_ICONWARNING );
   } else if ( logLevel == LOG_LEVEL_ERROR ) {
      MessageBeep( MB_ICONERROR );   // No need to check for a result code
      MessageBoxA( ( ( sphMainWindow == NULL ) ? NULL : *sphMainWindow ), buffer.sBuf, appName, MB_OK | MB_ICONERROR );
   } else if ( logLevel == LOG_LEVEL_FATAL ) {
      MessageBeep( MB_ICONSTOP );    // No need to check for a result code
      MessageBoxA( ( ( sphMainWindow == NULL ) ? NULL : *sphMainWindow ), buffer.sBuf, appName, MB_OK | MB_ICONSTOP );
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
/// Note: The logger **can** be called before it's initialized.  This is by
///       design.  The message box windows just won't have a parent window.
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
      MessageBoxW( ( ( sphMainWindow == NULL ) ? NULL : *sphMainWindow ), buffer.sBuf, appName, MB_OK | MB_ICONWARNING );
   } else if ( logLevel == LOG_LEVEL_ERROR ) {
      MessageBoxW( ( ( sphMainWindow == NULL ) ? NULL : *sphMainWindow ), buffer.sBuf, appName, MB_OK | MB_ICONERROR );
   } else if ( logLevel == LOG_LEVEL_FATAL ) {
      MessageBoxW( ( ( sphMainWindow == NULL ) ? NULL : *sphMainWindow ), buffer.sBuf, appName, MB_OK | MB_ICONSTOP );
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
   LOG_WARN(    "Narrow: 89012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123" );
   LOG_WARN(    "Narrow: 890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234" );

   LOG_WARN_W( L"Wide: 6789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123" );
   LOG_WARN_W( L"Wide: 67890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234" );

   // The following excessively long lines will fail an assertion check in vsprintf_s
   // This takes into account that "logTest: " is 10 characters long
//   LOG_INFO(    "Narrow: 8901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345" );
//   LOG_INFO_W( L"Wide: 678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345" );
}


/// The ID when no message has been set
#define NO_MESSAGE 0


static UINT suMsgId = NO_MESSAGE;  ///< The ID of the first valid message
static logLevels_t sMsgLevel = LOG_LEVEL_TRACE;  ///< The level of the first valid message


/// Save a message in the logger.
///
/// Only save messages where the severity level is #LOG_LEVEL_WARN,
/// #LOG_LEVEL_ERROR or #LOG_LEVEL_FATAL.  This is because these messages
/// generate `MessageBox`es and can't be displayed by worker threads or when
/// we have issues in the actual message loop.  So we save them and print the
/// first one we encounter at a later time.
///
/// This is not thread-safe, but it's close enough for what we are doing.
///
/// @param level The severity of the message
/// @param msgId The ID of the message
void logSetMsg( _In_ const logLevels_t level, _In_ const UINT msgId ) {
   _ASSERTE( msgId != NO_MESSAGE );

   if ( logHasMsg() )
      return;           /// Do nothing if a message has already been set

   if ( level < LOG_LEVEL_WARN )
      return;           /// Silently return if level is `<` #LOG_LEVEL_WARN

   suMsgId = msgId;
   sMsgLevel = level;
}


/// Get the ID of the first message
///
/// @return The ID of the first message.  If there are no messages, return
///         #NO_MESSAGE
UINT logGetMsgId() {
   return suMsgId;
}


/// Get the level of the first message
///
/// @return The level of the first message.  If there are no messages, return
///         #LOG_LEVEL_TRACE
logLevels_t logGetMsgLevel() {
   return sMsgLevel;
}


/// Reset the message store
void logResetMsg() {
   suMsgId    = NO_MESSAGE;
   sMsgLevel = LOG_LEVEL_TRACE;

   _ASSERTE( !logHasMsg() );
}


/// Report if the logger has a message
///
/// @return `true` if there's a message waiting.  `false` if there is not.
bool logHasMsg() {
   if( suMsgId == NO_MESSAGE ) {
      return false;
   } else {
      return true;
   }
}
