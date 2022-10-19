// header.h : include file for standard system include files,
// or project specific include files
//

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
