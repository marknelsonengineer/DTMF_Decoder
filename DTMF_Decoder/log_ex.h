///////////////////////////////////////////////////////////////////////////////
//          University of Hawaii, College of Engineering
//          DTMF_Decoder - EE 469 - Fall 2022
//
//  A Windows Desktop C program that decodes DTMF tones
//
/// Generic Windows logging utility (headers for extensions to the log)
///
/// @file    log_ex.h
/// @author  Mark Nelson <marknels@hawaii.edu>
///////////////////////////////////////////////////////////////////////////////

#pragma once

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


// Make the data passed into #logInit available to extensions
extern HWND*      sphMainWindow;
extern HINSTANCE* sphInstance;
extern CHAR       sAppName[ MAX_LOG_STRING ];
extern WCHAR      swAppName[ MAX_LOG_STRING ];
extern WCHAR      swAppTitle[ MAX_LOG_STRING ];


extern void logGetStringFromResources(
   _In_   const UINT       resourceId,
   _Inout_      wBuffer_t* pString );

extern void logShowMessageW( logLevels_t logLevel, PCWSTR message );

extern int vLogComposeW(
   _In_    const logLevels_t logLevel,
   _In_z_  const PCWSTR      functionName,
   _In_z_  const PCWSTR      format,
   _Inout_       wBuffer_t*  pBuffer,
   _In_    const va_list     args );

extern void vLogW(
   _In_       const logLevels_t logLevel,
   _In_z_     const PCWSTR  functionName,
   _In_opt_z_ const PCWSTR  resourceName,
   _In_       const UINT    resourceId,
   _In_z_     const PCWSTR  format,
   _In_z_     const va_list args );

/// Process an error within the logging subsystem
#define FATAL_IN_LOG( message )    \
   OutputDebugStringW( message );  \
   _ASSERT_EXPR( FALSE, message );
