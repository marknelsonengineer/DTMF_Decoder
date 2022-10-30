///////////////////////////////////////////////////////////////////////////////
//          University of Hawaii, College of Engineering
//          DTMF_Decoder - EE 469 - Fall 2022
//
//  A Windows Desktop C program that decodes DTMF tones
//
/// The implementation of the GDI and Direct2D paint commands as the view
/// component of this model-view-controller application.
///
/// @file    mvcView.h
/// @version 1.0
///
/// @author  Mark Nelson <marknels@hawaii.edu>
/// @date    10_Oct_2022
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <Windows.h>      // For BOOL, etc.


#define FOREGROUND_COLOR  (0x63B5FE)   /**< A light blue - courtesey of <a href="https://apps.apple.com/us/app/blue-box/id391832739">BlueBox</a>         */
#define HIGHLIGHT_COLOR   (0x75F0FF)   /**< A bright, light blue - courtesey of <a href="https://apps.apple.com/us/app/blue-box/id391832739">BlueBox</a> */
#define BACKGROUND_COLOR  (0x181737)   /**< A dark blue - courtesey of <a href="https://apps.apple.com/us/app/blue-box/id391832739">BlueBox</a>          */

BOOL mvcViewInitResources();
BOOL mvcViewCleanupResources();
BOOL mvcViewPaintWindow();
BOOL mvcViewRefreshWindow();

extern const int giWindowWidth;   ///< The overall width of the main window,
                                  ///< computed based on button size and spacing
extern const int giWindowHeight;  ///< The overall height of the main window
                                  ///< computed based on button size and spacing
