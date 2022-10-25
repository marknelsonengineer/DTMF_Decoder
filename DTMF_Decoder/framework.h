// header.h : include file for standard system include files,
// or project specific include files
//

///////////////////////////////////////////////////////////////////////////////
//          University of Hawaii, College of Engineering
//          DTMF_Decoder - EE 469 - Fall 2022
//
/// A Windows Desktop C program that decodes DTMF tones
/// 
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

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(P) if(P){P->Release() ; P = NULL ;}
#endif
