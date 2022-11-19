///////////////////////////////////////////////////////////////////////////////
//          University of Hawaii, College of Engineering
//          DTMF_Decoder - EE 469 - Fall 2022
//
//  A Windows Desktop C program that decodes DTMF tones
//
/// Include file for standard system include files or project specific include files
///
/// @file    framework.h
/// @author  Mark Nelson <marknels@hawaii.edu>
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "targetver.h"

/// @cond Doxygen_Suppress

// Disable large parts of the Windows API that we don't need
#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN

// Disable specific Windows APIs that we don't need
#define NOGDICAPMASKS
// #define NOVIRTUALKEYCODES  ///< We are looking for VK_ESCAPE
// #define NOWINMESSAGES      ///< We need the message loop API
// #define NOWINSTYLES        ///< We use some windows styles
#define NOSYSMETRICS
#define NOMENUS
#define NOICONS
#define NOKEYSTATES
#define NOSYSCOMMANDS
#define NORASTEROPS
#define NOSHOWWINDOW
#define OEMRESOURCE
#define NOATOM
#define NOCLIPBOARD
#define NOCOLOR
// #define NOCTLMGR           ///< Need the dialog box API
#define NODRAWTEXT
// #define NOGDI              ///< We use parts of the GDI API
#define NOKERNEL
// #define NOUSER             ///< Need the usual USERMODE API
#define NONLS
// #define NOMB               ///< Need for the logger's MessageBox
#define NOMEMMGR
#define NOMETAFILE
#define NOMINMAX
// #define NOMSG              ///< Need the messaging API
#define NOOPENFILE
#define NOSCROLL
#define NOSERVICE
#define NOSOUND               ///< We are NOT USING the native sound API
#define NOTEXTMETRIC
#define NOWH
#define NOWINOFFSETS
#define NOCOMM
#define NOKANJI
#define NOHELP
#define NOPROFILER
#define NODEFERWINDOWPOS
#define NOMCX
#define NOIME

/// @endcond


#include <Windows.h>      // For the standard Windows definitions
#include <crtdbg.h>       // For the _ASSERTE macro and _malloc_dbg

#include "DTMF_Decoder.h" // For APP_NAME
#include "log.h"          // For our logging functions
#include "resource.h"     // For the string table


/// Release the pointer P to a COM object by calling `IUnknown::Release`
/// and setting P to `NULL`
///
/// @see https://learn.microsoft.com/en-us/windows/win32/medfound/saferelease
/// @see https://learn.microsoft.com/en-us/windows/win32/api/unknwn/nf-unknwn-iunknown-release
///
#ifndef SAFE_RELEASE
   #define SAFE_RELEASE( P )                        \
      if( P != NULL ) {                             \
         P->Release();  /* Release does not return a meaningful result code */  \
         P = NULL;                                  \
      }
#endif


/// Post a custom message #guUMW_CLOSE_FATAL, passing in the resource string
/// ID and a number (usually a thread index).  Then, let the message handler
/// save the fatal error to the message log.
#define CLOSE_FATAL( resource_id, hiWord )        \
PostMessageA( ghMainWindow,                       \
              guUMW_CLOSE_FATAL,                  \
              MAKEWPARAM( resource_id, hiWord ),  \
              0 );


/// Standardized macro for processing a fatal error using a `printf`-style
/// resource string
///
/// - Set #giApplicationReturnValue to #EXIT_FAILURE
/// - Call #LOG_FATAL_R to display a message
///   - Log the message before we shutdown the message loop
/// - Call #gracefulShutdown
///
/// @see https://learn.microsoft.com/en-us/windows/win32/com/using-macros-for-error-handling
/// @see https://gcc.gnu.org/onlinedocs/cpp/Variadic-Macros.html
///
#define PROCESS_FATAL_R( resource_id, ... )  \
   giApplicationReturnValue = EXIT_FAILURE;  \
   LOG_FATAL_R( resource_id, __VA_ARGS__ );  \
   gracefulShutdown();


/// Standardized macro for processing a fatal error using a `printf`-style
/// resource string and returning `FALSE`
///
/// - Use #PROCESS_FATAL_R to tell the application about the problem
/// - Return `FALSE`
///
/// @see https://learn.microsoft.com/en-us/windows/win32/com/using-macros-for-error-handling
/// @see https://gcc.gnu.org/onlinedocs/cpp/Variadic-Macros.html
///
#define RETURN_FATAL_R( resource_id, ... )      \
   PROCESS_FATAL_R( resource_id, __VA_ARGS__ ); \
   return FALSE;


/// Standardized macro for checking the return value of COM functions that
/// return `HRESULT`s.  This macro uses resource strings and `printf`-style
/// varargs.
///
/// - If the check succeeds, continue processing
/// - If the check fails, start closing the application in failure mode and
///   return `FALSE`
///
/// @see https://learn.microsoft.com/en-us/windows/win32/com/using-macros-for-error-handling
/// @see https://gcc.gnu.org/onlinedocs/cpp/Variadic-Macros.html
///
#define CHECK_HR_R( resource_id, ... )             \
   if ( FAILED( hr ) ) {                           \
      RETURN_FATAL_R( resource_id, __VA_ARGS__ );  \
   }


/// Standardized macro for checking the return value of GDI functions that
/// return `BOOL`s.  This macro uses resource strings and `printf`-style
/// varargs.
///
/// - If the check succeeds, continue processing
/// - If the check fails, start closing the application in failure mode and
///   return `FALSE`
///
/// @see https://learn.microsoft.com/en-us/windows/win32/com/using-macros-for-error-handling
/// @see https://gcc.gnu.org/onlinedocs/cpp/Variadic-Macros.html
///
#define CHECK_BR_R( resource_id, ... )            \
   if ( !br ) {                                   \
      RETURN_FATAL_R( resource_id, __VA_ARGS__ ); \
   }


/// Standardized macro for checking the return value of GDI functions that
/// return `BOOL`s.  This macro uses resource strings and `printf`-style
/// varargs.
///
/// Failing a `WARN_` macro will just print a warning.  It won't change the program flow
///
/// @see https://gcc.gnu.org/onlinedocs/cpp/Variadic-Macros.html
///
#define WARN_BR_R( resource_id, ... )          \
   if ( !br ) {                                \
      LOG_WARN_R( resource_id, __VA_ARGS__ );  \
   }


/// Standardized macro for checking the return value of GDI functions that
/// return `BOOL`s.  For example,
///
/// - If the check succeeds, continue processing
/// - If the check fails, the macro will terminate the application via the
///   #guUMW_CLOSE_FATAL message.
///
/// @see https://learn.microsoft.com/en-us/windows/win32/com/using-macros-for-error-handling
///
#define CHECK_BR_C( resource_id, wParam )  \
   if ( !br ) {                            \
      CLOSE_FATAL( resource_id, wParam );  \
   }


/// Standardized macro for checking the return value of COM functions that
/// return `HRESULT`s.
///
/// - If the check succeeds, continue processing
/// - If the check fails, the macro will terminate the application via the
///   #guUMW_CLOSE_FATAL message.
///
/// @see https://learn.microsoft.com/en-us/windows/win32/com/using-macros-for-error-handling
///
#define CHECK_HR_C( resource_id, wParam )  \
   if ( FAILED( hr ) ) {                   \
      CLOSE_FATAL( resource_id, wParam );  \
   }


#ifdef _DEBUG
   /// When MONITOR_PCM_AUDIO is set, then DTMF_Decoder will monitor PCM
   /// data, tracking the maximum and minimum values.  Then, every 4 seconds,
   /// it will output the min/max values and then start over.  This capability
   /// helps us identify system noise and calibrate #GOERTZEL_MAGNITUDE_THRESHOLD.
   ///
   /// This is enabled in DEBUG versions and disabled in RELEASE versions.
#define MONITOR_PCM_AUDIO
#endif
