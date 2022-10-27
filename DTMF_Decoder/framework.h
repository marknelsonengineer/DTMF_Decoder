///////////////////////////////////////////////////////////////////////////////
//          University of Hawaii, College of Engineering
//          DTMF_Decoder - EE 469 - Fall 2022
//
//  A Windows Desktop C program that decodes DTMF tones
// 
/// Include file for standard system include files or project specific include files
/// 
/// @file framework.h
/// @version 1.0
///
/// @author Mark Nelson <marknels@hawaii.edu>
/// @date   10_Oct_2022
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "targetver.h"

#include <windows.h>                    // Windows Header Files

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>


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
#define CHECK_RESULT( message )                     \
   if ( FAILED( hr ) ) {                            \
      OutputDebugStringA( APP_NAME ": " message );  \
      return FALSE;                                 \
   }


/// Standardized macro for checking the result of GDI BOOL results.  For example,
/// If the function succeeds, the return value is nonzero.
/// If the function fails, the return value is zero.
#define CHECK_BOOL_RESULT( message )                \
   if ( !br ) {                                     \
      OutputDebugStringA( APP_NAME ": " message );  \
      return FALSE;                                 \
   }
