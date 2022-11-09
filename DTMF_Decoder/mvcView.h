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
/// @author  Mark Nelson <marknels@hawaii.edu>
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <Windows.h>      // For BOOL, etc.

#define FOREGROUND_COLOR  (0x63B5FE)   /**< A light blue - courtesey of <a href="https://apps.apple.com/us/app/blue-box/id391832739">BlueBox</a>         */
#define HIGHLIGHT_COLOR   (0x75F0FF)   /**< A bright, light blue - courtesey of <a href="https://apps.apple.com/us/app/blue-box/id391832739">BlueBox</a> */
#define BACKGROUND_COLOR  (0x181737)   /**< A dark blue - courtesey of <a href="https://apps.apple.com/us/app/blue-box/id391832739">BlueBox</a>          */

extern BOOL mvcViewInitResources();
extern BOOL mvcViewCleanupResources();
extern BOOL mvcViewPaintWindow(  _In_ const RECT*  pUpdateRect );

extern const int giWindowWidth;   ///< The overall width of the main window,
                                  ///< computed based on button size and spacing
extern const int giWindowHeight;  ///< The overall height of the main window
                                  ///< computed based on button size and spacing


#define BOX_WIDTH  (64)    /**< Width of each keypad button  */
#define BOX_HEIGHT (64)    /**< Height of each keypad button */

#define GAP_WIDTH  (16)    /**< Horizontal space between each keypad button */
#define GAP_HEIGHT (16)    /**< Vertical space between each keypad button   */

#define ROW0 (64)                                /**< Y-axis distance from the top for the 1st row */
#define ROW1 (ROW0 + BOX_HEIGHT + GAP_HEIGHT)    /**< Y-axis distance from the top for the 2nd row */
#define ROW2 (ROW1 + BOX_HEIGHT + GAP_HEIGHT)    /**< Y-axis distance from the top for the 3rd row */
#define ROW3 (ROW2 + BOX_HEIGHT + GAP_HEIGHT)    /**< Y-axis distance from the top for the 4th row */

#define COL0 (96)                                /**< X-axis distance from the left for the 1st column */
#define COL1 (COL0 + BOX_WIDTH + GAP_WIDTH)      /**< X-axis distance from the left for the 2nd column */
#define COL2 (COL1 + BOX_WIDTH + GAP_WIDTH)      /**< X-axis distance from the left for the 3rd column */
#define COL3 (COL2 + BOX_WIDTH + GAP_WIDTH)      /**< X-axis distance from the left for the 4th column */
