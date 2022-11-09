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

// Disable large parts of the Windows API that we do not need
#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN

// Disable specific Windows APIs that we do not need
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
// #define NOCTLMGR           /// Need the dialog box API
#define NODRAWTEXT
// #define NOGDI              /// We use parts of the GDI API
#define NOKERNEL
// #define NOUSER             /// Need the usual USERMODE API
#define NONLS
// #define NOMB               /// Need for the logger's MessageBox
#define NOMEMMGR
#define NOMETAFILE
#define NOMINMAX
// #define NOMSG              /// Need the messaging API
#define NOOPENFILE
#define NOSCROLL
#define NOSERVICE
#define NOSOUND               /// We are NOT USING the native sound API
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


/// Release the pointer P to a COM object by calling the IUnknown::Release
/// method and then setting P to NULL
///
/// @see https://learn.microsoft.com/en-us/windows/win32/medfound/saferelease
/// @see https://learn.microsoft.com/en-us/windows/win32/api/unknwn/nf-unknwn-iunknown-release
#ifndef SAFE_RELEASE
   #define SAFE_RELEASE( P )                        \
      if( P != NULL ) {                             \
         P->Release();  /* Release does not return a meaningful result code */  \
         P = NULL;                                  \
      }
#endif


/// Standardized macro for checking the result of COM HRESULT values
/// @see https://learn.microsoft.com/en-us/windows/win32/com/using-macros-for-error-handling
#define CHECK_HR( message ) \
   if ( FAILED( hr ) ) {    \
      LOG_FATAL( message ); \
      return FALSE;         \
   }


/// Standardized macro for checking the result of GDI BOOL results.  For example,
/// If the function succeeds, the return value is nonzero.
/// If the function fails, the return value is zero.
#define CHECK_BR( message ) \
   if ( !br ) {             \
      LOG_FATAL( message ); \
      return FALSE;         \
   }


/// Standardized macro for checking the result of GDI BOOL results.  For example,
/// If the function succeeds, the return value is nonzero.
/// If the function fails, the return value is zero.
///
/// Just print a warning.  Don't RETURN or change the program flow
#define WARN_BR( message )  \
   if ( !br ) {             \
      LOG_WARN( message );  \
   }


/// Standardized macro for checking the result of INT results.  For example,
/// If the function succeeds, the return value is nonzero.
/// If the function fails, the return value is zero.
#define CHECK_IR( message ) \
   if ( !ir ) {             \
      LOG_FATAL( message ); \
      return FALSE;         \
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
