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
///     also show a message box and appropriate icon
///   - Bounds checking on the string buffer
///   - Hold the buffer on the stack so it's both thread safe and re-entrant safe
///   - Append a `\n` because that's how the Windws debugger likes to print
///     output
///   - Support logging from resource strings
///   - Help thread and mesage loop handlers register an error to be displayed
///     at a later time (this mimics the Windows model of
///     [GetLastError](https://learn.microsoft.com/en-us/windows/win32/api/errhandlingapi/nf-errhandlingapi-getlasterror) and
///     [SetLastError](https://learn.microsoft.com/en-us/windows/win32/api/errhandlingapi/nf-errhandlingapi-setlasterror)
///     by queuing messages that can't be displayed
///     immediately  (from a worker thread or in a paint window message
///     handler) and play them back later (must be thread safe)
///
/// @see /// @see https://learn.microsoft.com/en-us/windows/win32/debug/error-handling
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
/// I'm also storing the a pointer to the instance handle as well.  This is
/// for getting strings from the resources via `LoadStringW`.  All of the
/// strings loaded from resources are assumed to be wide (UNICODE).
///
/// When Log functions throw exceptions, they will:
/// ````
///      OutputDebugStringA( "VIOLATED STACK GUARD in Logger.  Exiting immediately." );
///      _ASSERT_EXPR( FALSE, L"VIOLATED STACK GUARD in Logger.  Exiting immediately." );
/// ````
/// ... via #FATAL_IN_LOG
///
/// #### Queuing messages
/// Sometimes you need to log a message and display it later.  This happens
/// if you detect a warning, error or failure inside the message loop or in
/// one of the worker threads.  When this happens, you need to capture the event
/// and show the message later.  This is a common Windows design pattern...
///
/// To that end, we will use the same design pattern to save messages with
/// #LOG_INFO_Q, #logQ, #logQueueHasEntry, et. al..
///
/// The message IDs stored in this stateful holder should be the same IDs that
/// can be displayed by the `_R` loggers.
///
/// ## Generic Win32 API
/// | API                 | Link                                                                                                                              |
/// |---------------------| ----------------------------------------------------------------------------------------------------------------------------------|
/// | `LoadStringW`       | https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-loadstringw                                                |
///
/// ## Debugging& Instrumentation API
/// | API | Link                                                                                                                                               |
/// |----------------------| ----------------------------------------------------------------------------------------------------------------------------------|
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
/// ## CRT & Memory Management API
/// | API                | Link                                                                                         |
/// |--------------------| ---------------------------------------------------------------------------------------------|
/// | `CopyMemory`       | https://learn.microsoft.com/en-us/previous-versions/windows/desktop/legacy/aa366535(v=vs.85) |
/// | `SecureZeroMemory` | https://learn.microsoft.com/en-us/previous-versions/windows/desktop/legacy/aa366877(v=vs.85) |
/// | `rand`             | https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/rand?view=msvc-170         |
/// | `StringCchCopyW`   | https://learn.microsoft.com/en-us/windows/win32/api/strsafe/nf-strsafe-stringcchcopyw        |
///
/// ## Threads & Synchronization API
/// | API                         | Link                                                                                               |
/// |-----------------------------| ---------------------------------------------------------------------------------------------------|
/// | `InitializeCriticalSection` | https://learn.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-initializecriticalsection |
/// | `DeleteCriticalSection`     | https://learn.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-deletecriticalsection     |
/// | `EnterCriticalSection`      | https://learn.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-entercriticalsection      |
/// | `LeaveCriticalSection`      | https://learn.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-leavecriticalsection      |
///
/// @file    log.cpp
/// @author  Mark Nelson <marknels@hawaii.edu>
///////////////////////////////////////////////////////////////////////////////

#include "framework.h"    // Standard system include files
#include "log.h"          // For yourself

#include <stdio.h>        // For sprintf_s
#include <stdarg.h>       // For va_start
#include <strsafe.h>      // For StringCchCopy
#include <stdlib.h>       // For rand


/// Save a random number at the end of the string buffers.  When we're done,
/// make sure it's untouched.
///
/// If we overrun the buffer and violate the stack guard, then fail fast by
/// throwing an `_ASSERT_EXPR( FALSE, ...`.
#define BUFFER_GUARD 0xed539d63

/// Standard message string buffer with a guard at the end
struct wBuffer_t {
   WCHAR sBuf[ MAX_LOG_STRING ];  ///< A message string buffer
   DWORD dwGuard;                 ///< Guards the message in sBuf from overflowing
};

/// The ID when no resource has been identified.  When this is set, it is presumed
/// to be an ID to the string table in the resource file.
#define NO_RESOURCE 0

/// Pointer to an application's global windows handle.  This window will "own"
/// the message box popups.
static HWND* sphMainWindow = NULL;

/// Pointer to the application's current instance handle.  This is used to
/// lookup resources (strings) in the application.
static HINSTANCE* sphInst = NULL;

static CHAR  sAppName [ MAX_LOG_STRING ] = "";   ///< The (narrow) application name set in #logInit and used as the window title in `MessageBoxA`
static WCHAR swAppName[ MAX_LOG_STRING ] = L"";  ///< The (wide) application name set in #logInit and used as the window title in `MessageBoxW`


#define MAX_LOG_QUEUE_DEPTH 16     /**< The maximum depth of the log queue.  Set to `4` when testing. */
// #define MAX_LOG_QUEUE_DEPTH 4   /**< The maximum depth of the log queue (used for testing) */
static size_t log_queue_tail = 0;  ///< If the queue is not empty, then point to the first valid node in the queue.
static size_t log_queue_head = 0;  ///< If the queue is not full, then point to the next available entry in the queue.
static size_t log_queue_size = 0;  ///< The number of valid entries in the queue

static CRITICAL_SECTION log_queue_critical_section;  ///< Manage the queue in a thread-safe manner

/// An array of log messages that can't be displayed at the time the log is
/// generated (like in a worker thread or drawing a screen buffer).
static logEntry_t logQueue[ MAX_LOG_QUEUE_DEPTH ];

/// Process an error within the logging subsystem
#define FATAL_IN_LOG( message )   \
   OutputDebugStringW( message ); \
   _ASSERT_EXPR( FALSE, message );


/// Initialize the logger
///
/// The logger **can** be called before it's initialized.  This is by design...
/// `MessageBox` just won't have a parent window.
///
/// [Per Raymond Chen's book](https://devblogs.microsoft.com/oldnewthing/20050223-00/?p=36383),
/// all windows, including message boxes, __should__ be owned by their calling
/// window.  I don't want to pass an `hWnd` into each log (although, in the
/// future that may be necessary).  So, I'll initialize the logger and set
/// #sphMainWindow to point to the app's windows handle, which I expect the app
/// will keep up-to-date for me.  We use the same technique for the application's
/// instance handle `HINSTANCE`.
///
/// Don't use the resource string loggers (like #LOG_TRACE_R) before
/// initializing the logger as it won't have the application's `HINSTANCE` yet.
///
/// @param phInst   The application instance handle.  **This must be a global
///                 variable.**
///
/// @param phWindow The window that will own the log message boxes (usually the
///                 application's main window).  **This must be a global variable.**
///                 It is OK to set this to `NULL`
///
/// @param pAppName Save the name of the application so we don't have to pass
///                 it every time we log something.  It's OK to set this to
///                 `NULL` or an empty string.  This is used as the title for
///                 `MessageBoxA`.
///
/// @param pwAppName Same as pAppName but for wide characters and `MessageBoxW`
///
/// @return `TRUE` if successful.  `FALSE` if there was a problem.
BOOL logInit(
   _In_         HINSTANCE* phInst,
   _In_         HWND*      phWindow,
   _In_z_ const CHAR*      pAppName,
   _In_z_ const WCHAR*     pwAppName
) {

   /// The parameters are checked (validated) by #logValidate
   sphInst       = phInst;
   sphMainWindow = phWindow;

   HRESULT hr;

   if ( pAppName != NULL ) {
      hr = StringCchCopyA( sAppName, MAX_LOG_STRING, pAppName );
      if ( hr != S_OK ) {
         FATAL_IN_LOG( L"Failed to copy application name (narrow)" );
      }
   }

   if ( pwAppName != NULL ) {
      hr = StringCchCopyW( swAppName, MAX_LOG_STRING, pwAppName );
      if ( hr != S_OK ) {
         FATAL_IN_LOG( L"Failed to copy application name (wide)" );
      }
   }

   InitializeCriticalSection( &log_queue_critical_section );

   /// Call #logQueueReset to reset the message store
   logQueueReset();

   _ASSERTE( logValidate() );

   return TRUE;
}


/// Cleanup the logger
///
/// @return `TRUE` if successful.  `FALSE` if there was a problem.
BOOL logCleanup() {
   sphMainWindow = NULL;  /// Set #sphMainWindow to `NULL`

   /// No need to clean/erase #sphInst.  It should be good for the lifetime
   /// of the application.

   logQueueReset();

   DeleteCriticalSection( &log_queue_critical_section );

   return TRUE;
}


/// Generic narrow character, `printf`-style message logger
///
/// This is intended to be called through the logging macros like #LOG_INFO.
/// It is not intended to be called directly.
///
/// I'm choosing to make this a function rather than an inline.  Header-only
/// files are great, but I'd like to keep the buffer and all of the `vsprintf`
/// stuff in a self-contained library rather than push it into the caller's source.
///
/// The logger **can** be called before it's initialized.  This is by design...
/// `MessageBoxA` just won't have a parent window.
///
/// @param logLevel      The level of this logging event
/// @param functionName  The name of the function
/// @param format, ...   `printf`-style formatting
void logA(
   _In_   const logLevels_t logLevel,
   _In_z_ const CHAR*       functionName,
   _In_z_ const CHAR*       format,
   _In_ ... ) {
   /// Silently `return` if `functionName` or `format` is `NULL`
   if ( functionName == NULL || format == NULL )
      return;

   /// Silently `return` if `functionName` or `format` are empty
   if ( functionName[ 0 ] == '\0' || format[ 0 ] == '\0' )
      return;

   struct buffer_t {  // Narrow string buffer
      CHAR  sBuf[ MAX_LOG_STRING ];
      DWORD dwGuard;
   } buffer = { "", BUFFER_GUARD };

   va_list args;
   int     numCharsWritten   = 0;
   int     bufCharsRemaining = MAX_LOG_STRING;
   CHAR*   pBufferHead       = buffer.sBuf;

   numCharsWritten = sprintf_s( pBufferHead, bufCharsRemaining, "%s: ", functionName );
   // sprintf_s returns the number of characters written
   bufCharsRemaining -= numCharsWritten;
   pBufferHead       += numCharsWritten;

   /// Under normal circumstances, `vsprintf_s` will throw an `_ASSERT` failure
   /// on a buffer overflow (and it takes `\0` into account).
   va_start( args, format );  // va_start & va_end do not have result codes
   numCharsWritten = vsprintf_s( pBufferHead, bufCharsRemaining, format, args );
   va_end( args );
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
   /// throwing an `_ASSERT_EXPR( FALSE, ...`.
   if ( bufCharsRemaining < 0 || buffer.dwGuard != BUFFER_GUARD ) {
      FATAL_IN_LOG( L"VIOLATED STACK GUARD in Logger.  Exiting immediately." );
   }

   OutputDebugStringA( buffer.sBuf );

   if ( logLevel == LOG_LEVEL_WARN ) {
      MessageBoxA( ( ( sphMainWindow == NULL ) ? NULL : *sphMainWindow ), buffer.sBuf, sAppName, MB_OK | MB_ICONWARNING );
   } else if ( logLevel == LOG_LEVEL_ERROR ) {
      MessageBeep( MB_ICONERROR );   // No need to check for a result code
      MessageBoxA( ( ( sphMainWindow == NULL ) ? NULL : *sphMainWindow ), buffer.sBuf, sAppName, MB_OK | MB_ICONERROR );
   } else if ( logLevel == LOG_LEVEL_FATAL ) {
      MessageBeep( MB_ICONSTOP );    // No need to check for a result code
      MessageBoxA( ( ( sphMainWindow == NULL ) ? NULL : *sphMainWindow ), buffer.sBuf, sAppName, MB_OK | MB_ICONSTOP );
   }
}


/// Compose a wide character `vararg`-style message into a buffer
///
/// Centralized so there is a single version of truth (code) for
/// composing a message.  It is not intended to be called directly.
///
/// @param logLevel      The level of this logging event
/// @param functionName  The name of the function
/// @param format        `printf`-style formatting
/// @param pBuffer       Pointer to a pBuffer (the stack guard should be pre-populated or this will assert)
/// @param args          Varargs `va_list`
static int vLogComposeW(
   _In_    const logLevels_t logLevel,
   _In_z_  const WCHAR*      functionName,
   _In_z_  const WCHAR*      format,
   _Inout_       wBuffer_t*  pBuffer,
   _In_    const va_list     args ) {

   _ASSERTE( functionName != NULL );
   _ASSERTE( format != NULL );
   _ASSERTE( pBuffer != NULL );
   _ASSERTE( pBuffer->dwGuard == BUFFER_GUARD );

   _ASSERTE( functionName[ 0 ] != L'\0' );
   _ASSERTE( format[ 0 ]       != L'\0' );

   // There's no real validation for args

   int    numCharsWritten   = 0;
   int    bufCharsRemaining = MAX_LOG_STRING;
   WCHAR* pBufferHead       = pBuffer->sBuf;

   numCharsWritten = swprintf_s( pBufferHead, bufCharsRemaining, L"%s: ", functionName );
   // swprintf_s returns the number of characters written
   bufCharsRemaining -= numCharsWritten;
   pBufferHead       += numCharsWritten;

   /// Under normal circumstances, `vswprintf_s` will throw an `_ASSERT` failure
   /// on a buffer overflow (and it takes `\0` into account).  So, I'm not
   /// doing any return value checking of `vswprintf_s`
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
   /// throwing an `_ASSERT_EXPR( FALSE, ...`.
   if ( bufCharsRemaining < 0 || pBuffer->dwGuard != BUFFER_GUARD ) {
      FATAL_IN_LOG( L"VIOLATED STACK GUARD in Logger.  Exiting immediately." );
   }

   return numCharsWritten;
}


/// Get a string from the resource file
///
/// @param resourceId An ID from the string section of the resource file
/// @param pString    A guarded buffer to receive the string (#BUFFER_GUARD must be set before calling into this function)
static void logGetStringFromResources(
   _In_   const UINT       resourceId,
   _Inout_      wBuffer_t* pString ) {

   _ASSERTE( pString->dwGuard == BUFFER_GUARD );

   INT ir;  // INT result

   ir = LoadStringW( *sphInst, resourceId, pString->sBuf, MAX_LOG_STRING );
   /// Becuse we are in a log routine, it doesn't make sense to log an error
   /// message.  So, if there are problems, then fail fast by throwing an
   /// `_ASSERT_EXPR( FALSE, ...`.
   if ( !ir ) {
      FATAL_IN_LOG( L"Can't find string in resources.  Exiting immediately." );
   }

   /// If we overrun the format and violate the stack guard, then fail fast by
   /// throwing an `_ASSERT_EXPR( FALSE, ...`.
   if ( pString->dwGuard != BUFFER_GUARD ) {
      FATAL_IN_LOG( L"VIOLATED STACK GUARD in Logger.  Exiting immediately." );
   }
}


/// Display a log message via `MessageBoxW`
///
/// @param logLevel The level of the log message
/// @param message  The message to display
static void logShowMessageW( logLevels_t logLevel, WCHAR* message ) {
   _ASSERTE( message != NULL );

   switch ( logLevel ) {
      case LOG_LEVEL_WARN:
         MessageBoxW( ( ( sphMainWindow == NULL ) ? NULL : *sphMainWindow ), message, swAppName, MB_OK | MB_ICONWARNING );
         break;
      case LOG_LEVEL_ERROR:
         MessageBoxW( ( ( sphMainWindow == NULL ) ? NULL : *sphMainWindow ), message, swAppName, MB_OK | MB_ICONERROR );
         break;
      case LOG_LEVEL_FATAL:
         MessageBoxW( ( ( sphMainWindow == NULL ) ? NULL : *sphMainWindow ), message, swAppName, MB_OK | MB_ICONSTOP );
         break;
      default:
         FATAL_IN_LOG( L"Unexpected log level" );
   }
}


/// Generic wide character `vararg`-style message logger
///
/// This is intended to be called through the logging functions like #logW and
/// #logR.  It is not intended to be called directly.
///
/// I'm choosing to make this a function rather than an inline.  Header-only
/// files are great, but I'd like to keep the format and all of the `vsprintf`
/// stuff in a self-contained library rather than push it into the caller's source.
///
/// The logger **can** be called before it's initialized.  This is by design...
/// `MessageBoxW` just won't have a parent window.
///
/// @param logLevel      The level of this logging event
/// @param functionName  The name of the function
/// @param format        `printf`-style formatting
/// @param args          Varargs `va_list`
static void vLogW(
   _In_   const logLevels_t logLevel,
   _In_z_ const WCHAR*      functionName,
   _In_z_ const WCHAR*      format,
   _In_z_ const va_list     args ) {

   _ASSERTE( functionName != NULL );
   _ASSERTE( format       != NULL );

   _ASSERTE( functionName[ 0 ] != L'\0' );
   _ASSERTE( format[ 0 ]       != L'\0' );

   // There's no real validation for args

   wBuffer_t buffer = { L"", BUFFER_GUARD };

   vLogComposeW( logLevel, functionName, format, &buffer, args );

   OutputDebugStringW( buffer.sBuf );

   if ( logLevel >= LOG_LEVEL_WARN ) {
      logShowMessageW( logLevel, buffer.sBuf );
   }
}


/// Generic wide character, `printf`-style message logger
///
/// This is intended to be called through the wide string logging macros like
/// #LOG_INFO_W.  It is not intended to be called directly.
///
/// The logger **can** be called before it's initialized.  This is by design...
/// `MessageBoxW` just won't have a parent window.
///
/// @param logLevel      The level of this logging event
/// @param functionName  The name of the function
/// @param format, ...   `printf`-style formatting
void logW(
   _In_   const logLevels_t logLevel,
   _In_z_ const WCHAR*      functionName,
   _In_z_ const WCHAR*      format,
   _In_  ... ) {

   /// Silently `return` if `functionName` or `format` is `NULL`
   if ( functionName == NULL || format == NULL )
      return;

   /// Silently `return` if `functionName` or `format` are empty
   if ( functionName[ 0 ] == L'\0' || format[ 0 ] == L'\0' )
      return;

   va_list args;
   va_start( args, format );    // va_start & va_end do not have result codes
   vLogW( logLevel, functionName, format, args );
   va_end( args );
}


/// Generic wide character, `printf`-style resource string logger
///
/// This should be called through the wide resource string logging macros like
/// #LOG_INFO_R.  It is not intended to be called directly.
///
/// Loggers like this (that use resources) **can not** be called before it's
/// initialized.  The application's instance handle needs to be set for
/// `LoadStringW` to work.
///
/// @param logLevel        The level of this logging event
/// @param functionName    The name of the function
/// @param resourceId, ... The ID of the resource string.  The string may contain `printf`-style formatting characters.
void logR(
   _In_   const logLevels_t logLevel,
   _In_z_ const WCHAR*      functionName,
   _In_   const UINT        resourceId,
   _In_ ... ) {

   _ASSERTE( sphInst != NULL );

   /// Silently `return` if `functionName` is `NULL`
   if ( functionName == NULL )
      return;

   /// Silently `return` if `functionName` is empty
   if ( functionName[ 0 ] == L'\0' )
      return;

   wBuffer_t format = { L"", BUFFER_GUARD };

   logGetStringFromResources( resourceId, &format );

   va_list args;
   va_start( args, resourceId );  // va_start & va_end do not have result codes
   vLogW( logLevel, functionName, format.sBuf, args );
   va_end( args );
}


/// Generic wide character, `printf`-style resource string logger (to a queue)
///
/// This is used at times when a `MessageBox` can't be displayed at the time
/// the log is generated (like in a worker thread or drawing a screen pBuffer).
/// At these times, the application can use this to:
///   1. Immediately send the message to `OutputDebugStringW`
///   2. Store the message in a static queue (up to #MAX_LOG_QUEUE_DEPTH) entries
///   3. When the app can display a `MessageBox`, play back the message
///
/// This is intended to be called through the wide resource string logging
/// macros like #LOG_INFO_Q.  It is not intended to be called directly.
///
/// The logger **can not** be called before it's initialized.  The
/// application's instance handle needs to be set for `LoadStringW` to work.
///
/// @param logLevel        The level of this logging event
/// @param functionName    The name of the function
/// @param resourceId, ... The ID of the resource string.  The string may contain `printf`-style formatting characters.
void logQ(
   _In_   const logLevels_t logLevel,
   _In_z_ const WCHAR*      functionName,
   _In_   const UINT        resourceId,
   _In_ ... ) {

   _ASSERTE( sphInst != NULL );

   /// Silently `return` if `functionName` is `NULL`
   if ( functionName == NULL )
      return;

   /// Silently `return` if `functionName` is empty
   if ( functionName[ 0 ] == L'\0' )
      return;

   wBuffer_t format = { L"", BUFFER_GUARD };

   logGetStringFromResources( resourceId, &format );

   wBuffer_t buffer = { L"", BUFFER_GUARD };

   va_list args;
   va_start( args, resourceId );  // va_start & va_end do not have result codes
   vLogComposeW( logLevel, functionName, format.sBuf, &buffer, args );
   va_end( args );

   OutputDebugStringW( buffer.sBuf );

   _ASSERTE( logValidate() );

   EnterCriticalSection( &log_queue_critical_section );

   /// Post a #LOG_DEBUG_R message if the queue maxes out
   if ( log_queue_size >= MAX_LOG_QUEUE_DEPTH ) {
      LOG_DEBUG_R( IDS_LOG_MAXED_OUT_QUEUE );  // "MAX_LOG_QUEUE_DEPTH reached.  No longer queuing new messages."
      LeaveCriticalSection( &log_queue_critical_section );
      return;
   }

   /// No need to queue #LOG_LEVEL_TRACE, #LOG_LEVEL_DEBUG, #LOG_LEVEL_INFO
   /// messages.
   if ( logLevel == LOG_LEVEL_TRACE || logLevel == LOG_LEVEL_DEBUG || logLevel == LOG_LEVEL_INFO ) {
      LeaveCriticalSection( &log_queue_critical_section );
      return;
   }

   _ASSERTE( log_queue_head < MAX_LOG_QUEUE_DEPTH );

   logQueue[ log_queue_head ].uResourceId = resourceId;
   logQueue[ log_queue_head ].logLevel = logLevel;

   CopyMemory( logQueue[ log_queue_head ].sBuf, buffer.sBuf, sizeof( buffer.sBuf ) );

   log_queue_size += 1;
   log_queue_head = ++log_queue_head % MAX_LOG_QUEUE_DEPTH;

   LeaveCriticalSection( &log_queue_critical_section );

   _ASSERTE( logValidate() );
}


/// Validate the health of the log's data structures
///
/// @return `true` if successful.  `false` if there was a problem.
bool logValidate() {
   /// To validate the `sphInst` and `sphWindow` parameters, we try to read one
   /// byte from the pointers.  If the pointers are invalid, they will __likely__
   /// throw a "Read access violation".  We use a `__try` block to catch this.
   ///
   /// @see https://learn.microsoft.com/en-us/cpp/cpp/structured-exception-handling-c-cpp?view=msvc-170
   if ( sphInst != NULL ) {
      __try {
         char tmpChar = *(char*) sphInst;
         (void) tmpChar;  // Suppress a compiler warning that tmpChar is not checked after this.  No code is generated.
      } __except ( EXCEPTION_EXECUTE_HANDLER ) {
         FATAL_IN_LOG( L"The log's instance handle points to an invalid memory region.  Exiting immediately." );
      }
   } else {
      FATAL_IN_LOG(  L"The log's instance handle is NULL.  Exiting immediately." );
   }

   if ( sphMainWindow != NULL ) {
      __try {
         char tmpChar = *(char*) sphMainWindow;
         (void) tmpChar;  // Suppress a compiler warning that tmpChar is not checked after this.  No code is generated.
      } __except ( EXCEPTION_EXECUTE_HANDLER ) {
         FATAL_IN_LOG(  L"The log's window handle points to an invalid memory region.  Exiting immediately." );
      }
   }

   EnterCriticalSection( &log_queue_critical_section );

   _ASSERTE( log_queue_head < MAX_LOG_QUEUE_DEPTH );
   _ASSERTE( log_queue_tail < MAX_LOG_QUEUE_DEPTH );
   _ASSERTE( log_queue_size <= MAX_LOG_QUEUE_DEPTH );

   // If #log_queue_size == 0, then #log_queue_head must equal #log_queue_tail
   if ( log_queue_size == 0 ) {
      _ASSERTE( log_queue_head == log_queue_tail );
   }

   // If #log_queue_size == #MAX_LOG_QUEUE_DEPTH, then #log_queue_head must also equal #log_queue_tail
   if ( log_queue_size == MAX_LOG_QUEUE_DEPTH ) {
      _ASSERTE( log_queue_head == log_queue_tail );
   }

   if ( log_queue_tail < log_queue_head ) {
      long long calculated_size = (long long) log_queue_head - log_queue_tail;
      _ASSERTE( log_queue_size == calculated_size );
   } else if( log_queue_tail > log_queue_head ) {
      long long calculated_size = MAX_LOG_QUEUE_DEPTH - ( log_queue_tail - log_queue_head );
      _ASSERTE( log_queue_size == calculated_size );
   }

   for ( size_t i = 0 ; i < MAX_LOG_QUEUE_DEPTH ; i++ ) {
      _ASSERTE( logQueue[ i ].dwGuard == BUFFER_GUARD );
   }

   size_t count = MAX_LOG_QUEUE_DEPTH;
   size_t index = log_queue_tail;
   size_t remaining = log_queue_size;

   while ( count > 0 ) {
      if ( remaining > 0 ) {
         _ASSERTE( logQueue[ index ].logLevel    >= LOG_LEVEL_WARN );
         _ASSERTE( logQueue[ index ].uResourceId != NO_RESOURCE );
         _ASSERTE( logQueue[ index ].sBuf[ 0 ]   != L'\0' );

         remaining -= 1;
      } else {
         _ASSERTE( logQueue[ index ].logLevel    == LOG_LEVEL_TRACE );
         _ASSERTE( logQueue[ index ].uResourceId == NO_RESOURCE );
         _ASSERTE( logQueue[ index ].sBuf[ 0 ]   == L'\0' );
      }
      index = ++index % MAX_LOG_QUEUE_DEPTH;
      count -= 1;
   }

   LeaveCriticalSection( &log_queue_critical_section );

   return true;
}


/// Zero out an entry in the log queue
///
/// @param index The entry to zero out
void logQueueResetEntry( size_t index ) {
   logQueue[ index ].dwGuard = BUFFER_GUARD;
   SecureZeroMemory( logQueue[ index ].sBuf, sizeof( logQueue[ index ].sBuf ) );
   logQueue[ index ].logLevel = LOG_LEVEL_TRACE;
   logQueue[ index ].uResourceId = NO_RESOURCE;

   _ASSERTE( logQueue[ index ].dwGuard == BUFFER_GUARD );
}


/// Initialize / reset the log quueue
void logQueueReset() {

   EnterCriticalSection( &log_queue_critical_section );

   for ( size_t i = 0 ; i < MAX_LOG_QUEUE_DEPTH ; i++ ) {
      logQueueResetEntry( i );
   }

   log_queue_head = 0;
   log_queue_tail = 0;
   log_queue_size = 0;

   LeaveCriticalSection( &log_queue_critical_section );

   _ASSERTE( logValidate() );
   _ASSERTE( !logQueueHasEntry() );
}


/// Does the log queue have a message?
///
/// @return `true` if there's a log entry waiting.  `false` if there is not.
bool logQueueHasEntry() {
   if ( log_queue_size == 0 ) {
      return false;
   } else {
      return true;
   }
}


/// The size of the log queue
/// @return The size of the log queue
size_t logQueueSize() {
   return log_queue_size;
}


/// Dequeue a log message
///
/// @return The size of the queue after dequeuing this message
size_t logDequeue() {

   /// Return 0 if the queue is empty
   if ( log_queue_size == 0 ) {
      return 0;
   }

   _ASSERTE( logValidate() );

   EnterCriticalSection( &log_queue_critical_section );

   logQueueResetEntry( log_queue_tail );
   log_queue_size -= 1;
   log_queue_tail = ++log_queue_tail % MAX_LOG_QUEUE_DEPTH;

   LeaveCriticalSection( &log_queue_critical_section );

   _ASSERTE( logValidate() );

   return log_queue_size;
}


/// Dequeue and display a queued log message
///
/// @return The size of the queue after dequeuing this message
size_t logDequeueAndDisplayMessage() {

   /// Return 0 if the queue is empty
   if ( log_queue_size == 0 ) {
      return 0;
   }

   EnterCriticalSection( &log_queue_critical_section );

   logShowMessageW( logQueue[ log_queue_tail ].logLevel, logQueue[ log_queue_tail ].sBuf );

   LeaveCriticalSection( &log_queue_critical_section );

   return logDequeue();
}


/// Peek at the first message that could be dequeued.
///
/// @param logEntry Copy the message information into this buffer
/// @return `TRUE` if successful.  `FALSE` if there is no message to look at.
BOOL logPeekQueuedMessage( logEntry_t* logEntry ) {
   _ASSERTE( logEntry != NULL );

   EnterCriticalSection( &log_queue_critical_section );

   if ( log_queue_size == 0 ) {
      LeaveCriticalSection( &log_queue_critical_section );
      return FALSE;
   }

   CopyMemory( logEntry, &logQueue[ log_queue_tail ], sizeof( logEntry_t ) );

   LeaveCriticalSection( &log_queue_critical_section );

   return TRUE;
}


/// Test the logging functionality
///
/// To run all of the queue tests, set #MAX_LOG_QUEUE_DEPTH to 4
///
/// This is not normally used, except for testing.
///
void logTest() {

   // Narrow log testing
   LOG_TRACE( "Testing LOG_TRACE (narrow)" );
   LOG_DEBUG( "Testing LOG_DEBUG (narrow)" );
   LOG_INFO(  "Testing LOG_INFO (narrow)"  );
   LOG_WARN(  "Testing LOG_WARN (narrow)"  );
   LOG_ERROR( "Testing LOG_ERROR (narrow)" );
   LOG_FATAL( "Testing LOG_FATAL (narrow)" );

   // Narrow log testing with parameters
   LOG_TRACE( "Testing LOG_TRACE (narrow) varargs [%d] [%s] [%f]", 1, "TRACE", 1.0 );
   LOG_DEBUG( "Testing LOG_DEBUG (narrow) varargs [%d] [%s] [%f]", 2, "DEBUG", 2.0 );
   LOG_INFO(  "Testing LOG_INFO  (narrow) varargs [%d] [%s] [%f]", 3, "INFO",  3.0 );
   LOG_WARN(  "Testing LOG_WARN  (narrow) varargs [%d] [%s] [%f]", 4, "WARN",  4.0 );
   LOG_ERROR( "Testing LOG_ERROR (narrow) varargs [%d] [%s] [%f]", 5, "ERROR", 5.0 );
   LOG_FATAL( "Testing LOG_FATAL (narrow) varargs [%d] [%s] [%f]", 6, "FATAL", 6.0 );

   // Wide log testing
   LOG_TRACE_W( L"Testing LOG_TRACE_W (wide)" );
   LOG_DEBUG_W( L"Testing LOG_DEBUG_W (wide)" );
   LOG_INFO_W(  L"Testing LOG_INFO_W (wide)"  );
   LOG_WARN_W(  L"Testing LOG_WARN_W (wide)"  );
   LOG_ERROR_W( L"Testing LOG_ERROR_W (wide)" );
   LOG_FATAL_W( L"Testing LOG_FATAL_W (wide)" );

   // Wide log testing with parameters
   LOG_TRACE_W( L"Testing LOG_TRACE_W (wide) varargs [%d] [%s] [%f]", 1, L"TRACE", 1.0 );
   LOG_DEBUG_W( L"Testing LOG_DEBUG_W (wide) varargs [%d] [%s] [%f]", 2, L"DEBUG", 2.0 );
   LOG_INFO_W(  L"Testing LOG_INFO_W  (wide) varargs [%d] [%s] [%f]", 3, L"INFO",  3.0 );
   LOG_WARN_W(  L"Testing LOG_WARN_W  (wide) varargs [%d] [%s] [%f]", 4, L"WARN",  4.0 );
   LOG_ERROR_W( L"Testing LOG_ERROR_W (wide) varargs [%d] [%s] [%f]", 5, L"ERROR", 5.0 );
   LOG_FATAL_W( L"Testing LOG_FATAL_W (wide) varargs [%d] [%s] [%f]", 6, L"FATAL", 6.0 );

   // Resource log testing (wide)
   LOG_TRACE_R( IDS_LOG_TEST_BASIC );  // Testing LOG_<Level>_R (wide)
   LOG_DEBUG_R( IDS_LOG_TEST_BASIC );  // Testing LOG_<Level>_R (wide)
   LOG_INFO_R(  IDS_LOG_TEST_BASIC );  // Testing LOG_<Level>_R (wide)
   LOG_WARN_R(  IDS_LOG_TEST_BASIC );  // Testing LOG_<Level>_R (wide)
   LOG_ERROR_R( IDS_LOG_TEST_BASIC );  // Testing LOG_<Level>_R (wide)
   LOG_FATAL_R( IDS_LOG_TEST_BASIC );  // Testing LOG_<Level>_R (wide)

   // Resource log testing (wide) with parameters
   LOG_TRACE_R( IDS_LOG_TEST_PARAMETERS, 1, L"TRACE", 1.0 );  // Testing LOG_<Level>_R (wide) varargs [%d] [%s] [%f]
   LOG_DEBUG_R( IDS_LOG_TEST_PARAMETERS, 2, L"DEBUG", 2.0 );  // Testing LOG_<Level>_R (wide) varargs [%d] [%s] [%f]
   LOG_INFO_R(  IDS_LOG_TEST_PARAMETERS, 3, L"INFO",  3.0 );  // Testing LOG_<Level>_R (wide) varargs [%d] [%s] [%f]
   LOG_WARN_R(  IDS_LOG_TEST_PARAMETERS, 4, L"WARN",  4.0 );  // Testing LOG_<Level>_R (wide) varargs [%d] [%s] [%f]
   LOG_ERROR_R( IDS_LOG_TEST_PARAMETERS, 5, L"ERROR", 5.0 );  // Testing LOG_<Level>_R (wide) varargs [%d] [%s] [%f]
   LOG_FATAL_R( IDS_LOG_TEST_PARAMETERS, 6, L"FATAL", 6.0 );  // Testing LOG_<Level>_R (wide) varargs [%d] [%s] [%f]


   #if MAX_LOG_QUEUE_DEPTH == 4
      // Queued log testing (wide) with parameters
      // This works best when MAX_LOG_QUEUE_DEPTH = 4 or 5
      LOG_TRACE_Q( IDS_LOG_TEST_PARAMETERS_Q, 1, L"Queued TRACE", 1.0 );  // Testing LOG_<Level>_Q (wide) varargs [%d] [%s] [%f]
      LOG_DEBUG_Q( IDS_LOG_TEST_PARAMETERS_Q, 2, L"Queued DEBUG", 2.0 );  // Testing LOG_<Level>_Q (wide) varargs [%d] [%s] [%f]
      LOG_INFO_Q(  IDS_LOG_TEST_PARAMETERS_Q, 3, L"Queued INFO",  3.0 );  // Testing LOG_<Level>_Q (wide) varargs [%d] [%s] [%f]
      _ASSERTE( !logQueueHasMsg() );
      _ASSERTE( logQueueSize() == 0 );
      logEntry_t logEntry = { NO_RESOURCE, LOG_LEVEL_TRACE, L"", BUFFER_GUARD };
      _ASSERTE( logPeekQueuedMessage( &logEntry ) == FALSE );

      LOG_WARN_Q(  IDS_LOG_TEST_PARAMETERS_Q, 4, L"Queued WARN (4 of 12) - 1, 2 and 3 were TRACE, DEBUG and INFO", 4.0 );   // Testing LOG_<Level>_Q (wide) varargs [%d] [%s] [%f]
      _ASSERTE( logQueueHasMsg() );
      _ASSERTE( logQueueSize() == 1 );
      _ASSERTE( logPeekQueuedMessage( &logEntry ) == TRUE );
      _ASSERTE( logEntry.dwGuard == BUFFER_GUARD );
      _ASSERTE( logEntry.logLevel == LOG_LEVEL_WARN );
      _ASSERTE( logEntry.uResourceId == IDS_LOG_TEST_PARAMETERS_Q );

      LOG_ERROR_Q( IDS_LOG_TEST_PARAMETERS_Q, 5, L"Queued ERROR (5 of 12)", 5.0 );  // Testing LOG_<Level>_Q (wide) varargs [%d] [%s] [%f]
      _ASSERTE( logQueueHasMsg() );
      _ASSERTE( logQueueSize() == 2 );
      LOG_FATAL_Q( IDS_LOG_TEST_PARAMETERS_Q, 6, L"Queued FATAL (6 of 12)", 6.0 );  // Testing LOG_<Level>_Q (wide) varargs [%d] [%s] [%f]
      _ASSERTE( logQueueHasMsg() );
      _ASSERTE( logQueueSize() == 3 );

      _ASSERTE( logDequeueAndDisplayMessage() == 2 );
      _ASSERTE( logQueueHasMsg() );
      _ASSERTE( logQueueSize() == 2 );

      _ASSERTE( logDequeueAndDisplayMessage() == 1 );
      _ASSERTE( logQueueHasMsg() );
      _ASSERTE( logQueueSize() == 1 );

      _ASSERTE( logDequeueAndDisplayMessage() == 0 );
      _ASSERTE( !logQueueHasMsg() );
      _ASSERTE( logQueueSize() == 0 );

      LOG_WARN_Q( IDS_LOG_TEST_PARAMETERS_Q, 7, L"Queued WARN (7 of 12)", 7.0 );   // Testing LOG_<Level>_Q (wide) varargs [%d] [%s] [%f]
      _ASSERTE( logQueueHasMsg() );
      _ASSERTE( logQueueSize() == 1 );

      LOG_ERROR_Q( IDS_LOG_TEST_PARAMETERS_Q, 8, L"Queued ERROR (8 of 12)", 8.0 );   // Testing LOG_<Level>_Q (wide) varargs [%d] [%s] [%f]
      _ASSERTE( logQueueHasMsg() );
      _ASSERTE( logQueueSize() == 2 );

      LOG_FATAL_Q( IDS_LOG_TEST_PARAMETERS_Q, 9, L"Queued FATAL (9 of 12)", 9.0 );   // Testing LOG_<Level>_Q (wide) varargs [%d] [%s] [%f]
      _ASSERTE( logQueueHasMsg() );
      _ASSERTE( logQueueSize() == 3 );

      _ASSERTE( logDequeueAndDisplayMessage() == 2 );
      _ASSERTE( logQueueHasMsg() );
      _ASSERTE( logQueueSize() == 2 );

      _ASSERTE( logDequeueAndDisplayMessage() == 1 );
      _ASSERTE( logQueueHasMsg() );
      _ASSERTE( logQueueSize() == 1 );

      LOG_WARN_Q( IDS_LOG_TEST_PARAMETERS_Q, 10, L"Queued WARN (10 of 12)", 10.0 );   // Testing LOG_<Level>_Q (wide) varargs [%d] [%s] [%f]
      _ASSERTE( logQueueHasMsg() );
      _ASSERTE( logQueueSize() == 2 );

      LOG_ERROR_Q( IDS_LOG_TEST_PARAMETERS_Q, 11, L"Queued ERROR (11 of 12)", 11.0 );   // Testing LOG_<Level>_Q (wide) varargs [%d] [%s] [%f]
      _ASSERTE( logQueueHasMsg() );
      _ASSERTE( logQueueSize() == 3 );

      LOG_FATAL_Q( IDS_LOG_TEST_PARAMETERS_Q, 12, L"Queued FATAL (12 of 12)", 12.0 );   // Testing LOG_<Level>_Q (wide) varargs [%d] [%s] [%f]
      _ASSERTE( logQueueHasMsg() );
      _ASSERTE( logQueueSize() == 4 );

      // Try to enqueue when the queue is full
      LOG_WARN_Q( IDS_LOG_TEST_PARAMETERS_Q, 13, L"Attempt to Queue (queue full) - YOU SHOULD NOT SEE THIS", 13.0 );   // Testing LOG_<Level>_Q (wide) varargs [%d] [%s] [%f]
      _ASSERTE( logQueueHasMsg() );
      _ASSERTE( logQueueSize() == 4 );

      _ASSERTE( logDequeueAndDisplayMessage() == 3 );
      _ASSERTE( logQueueHasMsg() );
      _ASSERTE( logQueueSize() == 3 );

      _ASSERTE( logDequeueAndDisplayMessage() == 2 );
      _ASSERTE( logQueueHasMsg() );
      _ASSERTE( logQueueSize() == 2 );

      _ASSERTE( logDequeueAndDisplayMessage() == 1 );
      _ASSERTE( logQueueHasMsg() );
      _ASSERTE( logQueueSize() == 1 );

      _ASSERTE( logDequeueAndDisplayMessage() == 0 );
      _ASSERTE( !logQueueHasMsg() );
      _ASSERTE( logQueueSize() == 0 );

      // Try to dequeue an empty buffer
      _ASSERTE( logDequeueAndDisplayMessage() == 0 );
      _ASSERTE( !logQueueHasMsg() );
      _ASSERTE( logQueueSize() == 0 );
   #endif

   // Bounds testing... the following lines should succeed
   LOG_WARN(    "Narrow: 89012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123" );
   LOG_WARN(    "Narrow: 890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234" );

   LOG_WARN_W( L"Wide: 6789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123" );
   LOG_WARN_W( L"Wide: 67890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234" );

   LOG_WARN_R( IDS_LOG_TEST_253 );
   LOG_WARN_R( IDS_LOG_TEST_254 );

   // The following excessively long lines will fail an assertion check in vsprintf_s
   // This takes into account that "logTest: " is 10 characters long
   // LOG_INFO(    "Narrow: 8901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345" );
   // LOG_INFO_W( L"Wide: 678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345" );
   // LOG_INFO_R( IDS_LOG_TEST_255 );

   /// Queue stress test
   int size = 0;
   int totalOps = 0;
   for ( int i = 0 ; i < 100 ; i++ ) {
      int operation = rand() % 2;  // 0 is enqueue and 1 is dequeue

      for ( int numOps = rand() % MAX_LOG_QUEUE_DEPTH ; numOps > 0 ; numOps-- ) {
         _ASSERTE( logValidate() );

         if ( operation == 0 ) {
            LOG_WARN_Q( IDS_LOG_QUEUE_STRESS_TEST, size, totalOps );   // LOG_<Level>_Q (wide) stress test.  size=%zu  count=%zu
            size = ( size < MAX_LOG_QUEUE_DEPTH ) ? size + 1 : size;

            _ASSERTE( logQueueSize() == size );

         } else {
            logEntry_t logEntry = { NO_RESOURCE, LOG_LEVEL_TRACE, L"", BUFFER_GUARD };
            bool br = logPeekQueuedMessage( &logEntry );
            size_t x = logDequeue();

            if ( br ) {
               LOG_INFO_W( logEntry.sBuf, size, totalOps );
            } else {
               LOG_INFO_W( L"Dequeuing an empty queue.  count=%zu", totalOps );
            }

            size = ( size > 0 ) ? size - 1 : size;

            _ASSERTE( logQueueSize() == size );
            _ASSERTE( x == size );
         }
         totalOps++;
      }
   }
}


/// Save a message ID and level in the logger.
///
/// Only save messages where the severity level is #LOG_LEVEL_WARN,
/// #LOG_LEVEL_ERROR or #LOG_LEVEL_FATAL.  This is because these messages
/// generate `MessageBox`es and can't be displayed by worker threads or when
/// we have issues in the actual message loop.  So we save them and print the
/// first one we encounter at a later time.
///
/// This is not thread-safe, but it's close enough for what we are doing.
///
/// @param level     The severity of the message
/// @param msgId     The ID of the message
/// @param msgWParam A WPARAM passed into the message
void logSetMsg( _In_ const logLevels_t level, _In_ const UINT msgId, _In_ const WPARAM msgWParam ) {
}
