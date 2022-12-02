///////////////////////////////////////////////////////////////////////////////
//          University of Hawaii, College of Engineering
//          DTMF_Decoder - EE 469 - Fall 2022
//
//  A Windows Desktop C program that decodes DTMF tones
//
/// Generic Windows logging utility
///
/// @file    log.h
/// @author  Mark Nelson <marknels@hawaii.edu>
///////////////////////////////////////////////////////////////////////////////

#pragma once

/// Logging sybsystem log levels
enum logLevels_t {
   LOG_LEVEL_TRACE = 0,  ///< Log at TRACE level.
   LOG_LEVEL_DEBUG,      ///< Log at DEBUG level.
   LOG_LEVEL_INFO,       ///< Log at INFO level.
   LOG_LEVEL_WARN,       ///< Log at WARN level (and display a DialogBox).
   LOG_LEVEL_ERROR,      ///< Log at ERROR level (and display a DialogBox).
   LOG_LEVEL_FATAL       ///< Log at FATAL level (and display a DialogBox).
};


/// The maximum length of a log entry.  Anything longer will be truncated.
/// Logs will have a `\n` appended to them (for the Windows debugger)
#define MAX_LOG_STRING 256


/// A log message that can be queued and displayed later
struct logEntry_t {
   UINT        uResourceId;             ///< The resoruce string ID of the log entry
   logLevels_t logLevel;                ///< The level of the log entry
   WCHAR       sBuf[ MAX_LOG_STRING ];  ///< A queued log message
   DWORD       dwGuard;                 ///< Guards the message in sBuf
};


/// Log a narrow `printf`-style message at the the #LOG_LEVEL_TRACE level
#define LOG_TRACE( format, ... ) logA( LOG_LEVEL_TRACE, __FUNCTION__, format, __VA_ARGS__ )

/// Log a narrow `printf`-style message at the the #LOG_LEVEL_DEBUG level
#define LOG_DEBUG( format, ... ) logA( LOG_LEVEL_DEBUG, __FUNCTION__, format, __VA_ARGS__ )

/// Log a narrow `printf`-style message at the the #LOG_LEVEL_INFO level
#define LOG_INFO( format, ... )  logA( LOG_LEVEL_INFO,  __FUNCTION__, format, __VA_ARGS__ )

/// Log a narrow `printf`-style message at the the #LOG_LEVEL_WARN level
#define LOG_WARN( format, ... )  logA( LOG_LEVEL_WARN,  __FUNCTION__, format, __VA_ARGS__ )

/// Log a narrow `printf`-style message at the the #LOG_LEVEL_ERROR level
#define LOG_ERROR( format, ... ) logA( LOG_LEVEL_ERROR, __FUNCTION__, format, __VA_ARGS__ )

/// Log a narrow `printf`-style message at the the #LOG_LEVEL_FATAL level
#define LOG_FATAL( format, ... ) logA( LOG_LEVEL_FATAL, __FUNCTION__, format, __VA_ARGS__ )


/// Log a wide `printf`-style message at the the #LOG_LEVEL_TRACE level
#define LOG_TRACE_W( format, ... ) logW( LOG_LEVEL_TRACE, __FUNCTIONW__, format, __VA_ARGS__ )

/// Log a wide `printf`-style message at the the #LOG_LEVEL_DEBUG level
#define LOG_DEBUG_W( format, ... ) logW( LOG_LEVEL_DEBUG, __FUNCTIONW__, format, __VA_ARGS__ )

/// Log a wide `printf`-style message at the the #LOG_LEVEL_INFO level
#define LOG_INFO_W( format, ... )  logW( LOG_LEVEL_INFO,  __FUNCTIONW__, format, __VA_ARGS__ )

/// Log a wide `printf`-style message at the the #LOG_LEVEL_WARN level
#define LOG_WARN_W( format, ... )  logW( LOG_LEVEL_WARN,  __FUNCTIONW__, format, __VA_ARGS__ )

/// Log a wide `printf`-style message at the the #LOG_LEVEL_ERROR level
#define LOG_ERROR_W( format, ... ) logW( LOG_LEVEL_ERROR, __FUNCTIONW__, format, __VA_ARGS__ )

/// Log a wide `printf`-style message at the the #LOG_LEVEL_FATAL level
#define LOG_FATAL_W( format, ... ) logW( LOG_LEVEL_FATAL, __FUNCTIONW__, format, __VA_ARGS__ )


/// Log a wide `printf`-style resource string at the the #LOG_LEVEL_TRACE level
#define LOG_TRACE_R( uId, ... ) logR( LOG_LEVEL_TRACE, __FUNCTIONW__, L"" #uId, uId, __VA_ARGS__ )

/// Log a wide `printf`-style resource string at the the #LOG_LEVEL_DEBUG level
#define LOG_DEBUG_R( uId, ... ) logR( LOG_LEVEL_DEBUG, __FUNCTIONW__, L"" #uId, uId, __VA_ARGS__ )

/// Log a wide `printf`-style resource string at the the #LOG_LEVEL_INFO level
#define LOG_INFO_R( uId, ... )  logR( LOG_LEVEL_INFO,  __FUNCTIONW__, L"" #uId, uId, __VA_ARGS__ )

/// Log a wide `printf`-style resource string at the the #LOG_LEVEL_WARN level
#define LOG_WARN_R( uId, ... )  logR( LOG_LEVEL_WARN,  __FUNCTIONW__, L"" #uId, uId, __VA_ARGS__ )

/// Log a wide `printf`-style resource string at the the #LOG_LEVEL_ERROR level
#define LOG_ERROR_R( uId, ... ) logR( LOG_LEVEL_ERROR, __FUNCTIONW__, L"" #uId, uId, __VA_ARGS__ )

/// Log a wide `printf`-style resource string at the the #LOG_LEVEL_FATAL level
#define LOG_FATAL_R( uId, ... ) logR( LOG_LEVEL_FATAL, __FUNCTIONW__, L"" #uId, uId, __VA_ARGS__ )


/// Queue a wide `printf`-style resource string at the the #LOG_LEVEL_TRACE level
#define LOG_TRACE_Q( uId, ... ) logQ( LOG_LEVEL_TRACE, __FUNCTIONW__, L"" #uId, uId, __VA_ARGS__ )

/// Queue a wide `printf`-style resource string at the the #LOG_LEVEL_DEBUG level
#define LOG_DEBUG_Q( uId, ... ) logQ( LOG_LEVEL_DEBUG, __FUNCTIONW__, L"" #uId, uId, __VA_ARGS__ )

/// Queue a wide `printf`-style resource string at the the #LOG_LEVEL_INFO level
#define LOG_INFO_Q( uId, ... )  logQ( LOG_LEVEL_INFO,  __FUNCTIONW__, L"" #uId, uId, __VA_ARGS__ )

/// Queue a wide `printf`-style resource string at the the #LOG_LEVEL_WARN level
#define LOG_WARN_Q( uId, ... )  logQ( LOG_LEVEL_WARN,  __FUNCTIONW__, L"" #uId, uId, __VA_ARGS__ )

/// Queue a wide `printf`-style resource string at the the #LOG_LEVEL_ERROR level
#define LOG_ERROR_Q( uId, ... ) logQ( LOG_LEVEL_ERROR, __FUNCTIONW__, L"" #uId, uId, __VA_ARGS__ )

/// Queue a wide `printf`-style resource string at the the #LOG_LEVEL_FATAL level
#define LOG_FATAL_Q( uId, ... ) logQ( LOG_LEVEL_FATAL, __FUNCTIONW__, L"" #uId, uId, __VA_ARGS__ )  // NOLINT


extern BOOL logInit(
   _In_         HINSTANCE* phInst,
   _In_         HWND*      phWindow,
   _In_z_ const CHAR*      pAppName,
   _In_z_ const WCHAR*     pwAppName,
   _In_z_ const WCHAR*     pwAppTitle

);

extern BOOL logCleanup();

extern void logA(
   _In_   const logLevels_t logLevel,
   _In_z_ const CHAR*       functionName,
   _In_z_ const CHAR*       format,
   _In_ ... );

extern void logW(
   _In_   const logLevels_t logLevel,
   _In_z_ const WCHAR*      functionName,
   _In_z_ const WCHAR*      format,
   _In_ ... );

extern void logR(
   _In_   const logLevels_t logLevel,
   _In_z_ const WCHAR*      functionName,
   _In_z_ const WCHAR*      resourceName,
   _In_   const UINT        resourceId,
   _In_   ... );

extern void logQ(
   _In_   const logLevels_t logLevel,
   _In_z_ const WCHAR*      functionName,
   _In_z_ const WCHAR*      resourceName,
   _In_   const UINT        resourceId,
   _In_   ... );

extern void logTest();

extern void   logQueueReset();
extern bool   logQueueHasEntry();
extern bool   logValidate();
extern size_t logQueueSize();

extern size_t logDequeueAndDisplayMessage();
extern BOOL   logPeekQueuedMessage( logEntry_t* logEntry );
