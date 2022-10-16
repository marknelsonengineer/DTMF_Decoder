///////////////////////////////////////////////////////////////////////////////
//          University of Hawaii, College of Engineering
//          DTMF_Decoder - EE 469 - Fall 2022
//
/// A Windows Desktop C program that decodes DTMF tones
///
/// @file mvcView.h
/// @version 1.0
///
/// @author Mark Nelson <marknels@hawaii.edu>
/// @date   10_Oct_2022
///////////////////////////////////////////////////////////////////////////////

#pragma once

#define FOREGROUND_COLOR  (0x63B5FE)
#define HIGHLIGHT_COLOR   (0x75F0FF)
#define BACKGROUND_COLOR  (0x181737)

BOOL mvcViewInitResources( HWND );
BOOL mvcViewCleanupResources();
BOOL mvcViewPaintWindow( HWND );
