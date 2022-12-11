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

#include "resource.h"     // For the string table
#include "DTMF_Decoder.h" // For APP_NAME
#include "log.h"          // For our logging functions


/// The application's return value.  This defaults to 0 (success).  Any error
/// handler can set this and it will be passed out of the program when it
/// terminates.
//  Included here to avoid circular .h references with mvcModel.h
extern int giApplicationReturnValue;


/// Add SAL notation when returning a `BOOL` in the usual
/// ````
///      /// @return TRUE if successful.  FALSE if there was a problem.
/// ````
/// ...design pattern.
#define RETURN_BOOL _Success_( return != 0 ) BOOL


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


/// Standardized macro for queuing a fatal error using a `printf`-style
/// resource string - Extended to include the resource name
#define QUEUE_FATAL_EX( resource_name, resource_id, ... )       \
   giApplicationReturnValue = EXIT_FAILURE;                     \
   LOG_FATAL_QX( resource_name, resource_id, __VA_ARGS__ );     \
   gracefulShutdown()


/// Standardized macro for queuing a fatal error using a `printf`-style
/// resource string
#define QUEUE_FATAL( resource_id, ... )                          \
   QUEUE_FATAL_EX( L"" #resource_id, resource_id, __VA_ARGS__ )  \


/// Standardized macro for processing a fatal error using a `printf`-style
/// resource string - Extended to include the resource name
///
/// @see https://learn.microsoft.com/en-us/windows/win32/com/using-macros-for-error-handling
/// @see https://gcc.gnu.org/onlinedocs/cpp/Variadic-Macros.html
///
#define PROCESS_FATAL_EX( resource_name, resource_id, ... )     \
   giApplicationReturnValue = EXIT_FAILURE;                     \
   LOG_FATAL_RX( resource_name, resource_id, __VA_ARGS__ );     \
   gracefulShutdown()


/// Standardized macro for processing a fatal error using a `printf`-style
/// resource string
///
/// @see https://learn.microsoft.com/en-us/windows/win32/com/using-macros-for-error-handling
/// @see https://gcc.gnu.org/onlinedocs/cpp/Variadic-Macros.html
///
#define PROCESS_FATAL( resource_id, ... )                           \
   PROCESS_FATAL_EX( L"" #resource_id, resource_id, __VA_ARGS__ );  \


/// Standardized macro for processing a fatal error using a `printf`-style
/// resource string and returning `FALSE` - Extended to include the resource name
///
/// @see https://learn.microsoft.com/en-us/windows/win32/com/using-macros-for-error-handling
/// @see https://gcc.gnu.org/onlinedocs/cpp/Variadic-Macros.html
///
#define RETURN_FATAL_EX( resource_name, resource_id, ... )          \
   PROCESS_FATAL_EX( resource_name, resource_id, __VA_ARGS__ );     \
   return FALSE


/// Standardized macro for processing a fatal error using a `printf`-style
/// resource string and returning `FALSE`
///
/// @see https://learn.microsoft.com/en-us/windows/win32/com/using-macros-for-error-handling
/// @see https://gcc.gnu.org/onlinedocs/cpp/Variadic-Macros.html
///
#define RETURN_FATAL( resource_id, ... )                            \
   PROCESS_FATAL_EX( L"" #resource_id, resource_id, __VA_ARGS__ );  \
   return FALSE


/// Standardized macro for checking the return value of functions that
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
#define CHECK_HR_R( resource_id, ... )                                \
   if ( FAILED( hr ) ) {                                              \
      RETURN_FATAL_EX( L"" #resource_id, resource_id, __VA_ARGS__ );  \
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
#define CHECK_BR_R( resource_id, ... )                                \
   if ( !br ) {                                                       \
      RETURN_FATAL_EX( L"" #resource_id, resource_id, __VA_ARGS__ );  \
   }


/// Standardized macro for checking the return value of GDI functions that
/// return `BOOL`s.  This macro uses resource strings and `printf`-style
/// varargs.
///
/// Failing a `WARN_` macro will just print a warning.  It won't change the
/// program flow
///
/// @see https://gcc.gnu.org/onlinedocs/cpp/Variadic-Macros.html
///
#define WARN_BR_R( resource_id, ... )            \
   if ( !br ) {                                  \
      LOG_WARN_R( resource_id, __VA_ARGS__ );    \
   }


/// Standardized macro for checking the return value of GDI functions that
/// return `BOOL`s and queuing any errors.  This macro uses resource strings
/// and `printf`-style varargs.
///
/// - If the check succeeds, continue processing
/// - If the check fails, queue a message and close the program
///
/// @see https://learn.microsoft.com/en-us/windows/win32/com/using-macros-for-error-handling
#define CHECK_BR_Q( resource_id, ... )                               \
   if ( !br ) {                                                      \
      QUEUE_FATAL_EX( L"" #resource_id, resource_id, __VA_ARGS__ );  \
   }


/// Standardized macro for checking the return value of functions that
/// return `HRESULT`s.  This macro uses resource strings and `printf`-style
/// varargs.
///
/// - If the check succeeds, continue processing
/// - If the check fails, queue a message and close the program
///
/// @see https://learn.microsoft.com/en-us/windows/win32/com/using-macros-for-error-handling
#define CHECK_HR_Q( resource_id, ... )                               \
   if ( FAILED( hr ) ) {                                             \
      QUEUE_FATAL_EX( L"" #resource_id, resource_id, __VA_ARGS__ );  \
   }


/// Standardized macro for checking the return value of functions that
/// return `HRESULT`s.  This macro uses resource strings and `printf`-style
/// varargs.
///
/// - If the check succeeds, continue processing
/// - If the check fails, print a warning but don't change the program flow
///
#define WARN_HR_R( resource_id, ... )          \
   if ( FAILED( hr ) ) {                       \
      LOG_WARN_R( resource_id, __VA_ARGS__ );  \
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
