///////////////////////////////////////////////////////////////////////////////
//          University of Hawaii, College of Engineering
//          DTMF_Decoder - EE 469 - Fall 2022
//
//  A Windows Desktop C program that decodes DTMF tones
// 
/// The implementation of the GDI and Direct2D paint commands as the view 
/// component of this model-view-controller application.
///
/// @file mvcView.cpp
/// @version 1.0
/// 
/// @see http://www.catch22.net/tuts/win32/flicker-free-drawing
///
/// @author Mark Nelson <marknels@hawaii.edu>
/// @date   10_Oct_2022
///////////////////////////////////////////////////////////////////////////////

#include "framework.h"    // Standard system include files
#include <d2d1.h>         // For Direct2D (drawing)
#include <dwrite.h>       // For DirectWrite (fonts and text)
#include "mvcView.h"      // For drawing the window
#include "mvcModel.h"     // For viewing the state of the machine
#include "DTMF_Decoder.h" // Resource.h
#include <cassert>

#pragma comment(lib, "d2d1")    // Link the Diect2D library (for drawing)
#pragma comment(lib, "Dwrite")  // Link the DirectWrite library (for fonts and text)

// Global Variables (private to this source file)
HWND                   gHwnd               = NULL;  ///< Handle to the main window
ID2D1Factory*          gpD2DFactory        = NULL;  ///< The Direct2D Factory
ID2D1HwndRenderTarget* gpRenderTarget      = NULL;	 ///< Render target
ID2D1SolidColorBrush*  gpBrushForeground   = NULL;  ///< A light blue brush for the foreground
ID2D1SolidColorBrush*  gpBrushHighlight    = NULL;  ///< A lighter blue brush for the highlight
ID2D1SolidColorBrush*  gpBrushBackground   = NULL;  ///< A dark blue brush for the background (not used right now)
IDWriteFactory*        gpDWriteFactory     = NULL;  ///< A DirectWrite factory object
IDWriteTextFormat*     gpDigitTextFormat   = NULL;  ///< The font for the digits
IDWriteTextFormat*     gpLettersTextFormat = NULL;  ///< The font for the letters above the digits (and the `Hz` units)
IDWriteTextFormat*     gpFreqTextFormat    = NULL;  ///< The font for the frequency

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

const int windowWidth = COL0 + ( BOX_WIDTH * 4 ) + ( GAP_WIDTH * 3 ) + BOX_WIDTH;
const int windowHeight = ROW0 + ( BOX_HEIGHT * 4 ) + ( GAP_HEIGHT * 3 ) + BOX_HEIGHT + 50;  // The title is 25px and the menu is 25px

/// Holds the location and display data for each key (button) on the keypad
typedef struct {
   WCHAR  digit[2];   ///< The digit to print
   size_t row;        ///< Row index into dtmfTones
   size_t column;     ///< Column index into dtmfTones
   WCHAR  letters[5]; ///< The letters above the digit
   float  x;          ///< The upper-left corner of the digit's box
   float  y;          ///< The upper-left corner of the digit's box
} keypad_t;


/// An array of information (location and display data) for each key (button)
/// on the keypad
keypad_t keypad[ 16 ] = {
   {L"1", 0, 4,     L"", COL0, ROW0 },
   {L"2", 0, 5,  L"ABC", COL1, ROW0 },
   {L"3", 0, 6,  L"DEF", COL2, ROW0 },
   {L"A", 0, 7,     L"", COL3, ROW0 },
   {L"4", 1, 4,  L"GHI", COL0, ROW1 },
   {L"5", 1, 5,  L"JKL", COL1, ROW1 },
   {L"6", 1, 6,  L"MNO", COL2, ROW1 },
   {L"B", 1, 7,     L"", COL3, ROW1 },
   {L"7", 2, 4, L"PQRS", COL0, ROW2 },
   {L"8", 2, 5,  L"TUV", COL1, ROW2 },
   {L"9", 2, 6, L"WXYZ", COL2, ROW2 },
   {L"C", 2, 7,     L"", COL3, ROW2 },
   {L"*", 3, 4,     L"", COL0, ROW3 },
   {L"0", 3, 5,     L"", COL1, ROW3 },
   {L"#", 3, 6,     L"", COL2, ROW3 },
   {L"D", 3, 7,     L"", COL3, ROW3 }
};


// Forward declarations of private functions in this file
BOOL paintKey( size_t index );
BOOL paintRowFreq( size_t index );
BOOL paintColFreq( size_t index );


/// Repaint the main window (keypad) -- probably because the state
/// of one of the buttons has changed
/// 
/// @return `true` if successful.  `false` if there was a problem.
BOOL mvcViewRefreshWindow() {
   BOOL    br;  // BOOL result

   br = InvalidateRect( gHwnd, NULL, FALSE );
   CHECK_BOOL_RESULT( "Failed to invalidate rectangle" );

   br = UpdateWindow( gHwnd );
   CHECK_BOOL_RESULT( "Failed to update window" );

   return TRUE;
}


/// Initialize all of the resources needed to draw the main window
/// 
/// @param hWnd Window handle
/// @return `true` if successful.  `false` if there was a problem.
BOOL mvcViewInitResources( HWND hWnd ) {
   HRESULT hr;  // HRESULT result
   BOOL    br;  // BOOL result

   gHwnd = hWnd;  // Save in global mvcViewRefreshWindow 

   /// Initialize Direct2D
   hr = D2D1CreateFactory( D2D1_FACTORY_TYPE_MULTI_THREADED, &gpD2DFactory );
   CHECK_RESULT( "Failed to create Direct2D Factory" );

   /// Initialize DirectWrite
   hr = DWriteCreateFactory(
      DWRITE_FACTORY_TYPE_SHARED,
      __uuidof( IDWriteFactory ),
      reinterpret_cast<IUnknown**>( &gpDWriteFactory )
   );
   CHECK_RESULT( "Failed to create DirectWrite Factory" );

   /// Create the font for the digits
   hr = gpDWriteFactory->CreateTextFormat(
      L"Segoe UI",                  // Font family name
      NULL,                         // Font collection(NULL sets it to the system font collection)
      DWRITE_FONT_WEIGHT_MEDIUM,    // Weight
      DWRITE_FONT_STYLE_NORMAL,     // Style
      DWRITE_FONT_STRETCH_NORMAL,   // Stretch
      36.0f,                        // Size	
      L"en-us",                     // Local
      &gpDigitTextFormat            // Pointer to recieve the created object
   );
   CHECK_RESULT( "Failed to create a font resource (digit text format)" );

   hr = gpDigitTextFormat->SetWordWrapping( DWRITE_WORD_WRAPPING_WRAP );
   CHECK_RESULT( "Failed to set word wrap mode (digit text format)" );

   hr = gpDigitTextFormat->SetTextAlignment( DWRITE_TEXT_ALIGNMENT_CENTER );  // Horizontal alignment
   CHECK_RESULT( "Failed to set text alignment (digit text format)" );

   hr = gpDigitTextFormat->SetParagraphAlignment( DWRITE_PARAGRAPH_ALIGNMENT_CENTER );  // Vertical alignment
   CHECK_RESULT( "Failed to set paragraph alighment (digit text format)" );

   /// Create the font for the letters
   hr = gpDWriteFactory->CreateTextFormat(
      L"Segoe UI",                  // Font family name
      NULL,                         // Font collection(NULL sets it to the system font collection)
      DWRITE_FONT_WEIGHT_REGULAR,   // Weight
      DWRITE_FONT_STYLE_NORMAL,     // Style
      DWRITE_FONT_STRETCH_NORMAL,   // Stretch
      16.0f,                        // Size	
      L"en-us",                     // Local
      &gpLettersTextFormat          // Pointer to recieve the created object
   );
   CHECK_RESULT( "Failed to create a font resource (letters text format)" );

   hr = gpLettersTextFormat->SetWordWrapping( DWRITE_WORD_WRAPPING_WRAP );
   CHECK_RESULT( "Failed to set word wrap mode (letters text format)" );
 
   hr = gpLettersTextFormat->SetTextAlignment( DWRITE_TEXT_ALIGNMENT_CENTER );  // Horizontal alignment
   CHECK_RESULT( "Failed to set text alignment (letters text format)" );

   hr = gpLettersTextFormat->SetParagraphAlignment( DWRITE_PARAGRAPH_ALIGNMENT_CENTER );  // Vertical alignment
   CHECK_RESULT( "Failed to set paragraph alighment (letters text format)" );

   /// Create the font for the frequency labels
   hr = gpDWriteFactory->CreateTextFormat(
      L"Segoe UI",                  // Font family name
      NULL,                         // Font collection(NULL sets it to the system font collection)
      DWRITE_FONT_WEIGHT_REGULAR,   // Weight
      DWRITE_FONT_STYLE_NORMAL,     // Style
      DWRITE_FONT_STRETCH_NORMAL,   // Stretch
      24.0f,                        // Size	
      L"en-us",                     // Local
      &gpFreqTextFormat             // Pointer to recieve the created object
   );
   CHECK_RESULT( "Failed to create a font resource (frequency text format)" );

   hr = gpFreqTextFormat->SetWordWrapping( DWRITE_WORD_WRAPPING_WRAP );
   CHECK_RESULT( "Failed to set word wrap mode (frequency text format)" );

   hr = gpFreqTextFormat->SetTextAlignment( DWRITE_TEXT_ALIGNMENT_TRAILING );  // Horizontal alignment
   CHECK_RESULT( "Failed to set text alignment (frequency text format)" );
   
   hr = gpFreqTextFormat->SetParagraphAlignment( DWRITE_PARAGRAPH_ALIGNMENT_CENTER );  // Vertical alignment
   CHECK_RESULT( "Failed to set paragraph alighment (frequency text format)" );

   /// Create the Direct2D render target
   RECT clientRectangle ;
   br = GetClientRect( hWnd, &clientRectangle );
   CHECK_BOOL_RESULT( "Failed to get the window size" );

   hr = gpD2DFactory->CreateHwndRenderTarget(
      D2D1::RenderTargetProperties(),
      D2D1::HwndRenderTargetProperties( hWnd, D2D1::SizeU( clientRectangle.right - clientRectangle.left, clientRectangle.bottom - clientRectangle.top ) ),
      &gpRenderTarget
   );
   CHECK_RESULT( "Failed to create Direct2D Render Target" );

   /// Create the colors (brushes) for the foreground, highlight and background
   hr = gpRenderTarget->CreateSolidColorBrush(
      D2D1::ColorF( D2D1::ColorF( FOREGROUND_COLOR, 1.0f ) ),
      &gpBrushForeground
   );
   CHECK_RESULT( "Failed to create Direct2D Brush (Foreground)" );

   hr = gpRenderTarget->CreateSolidColorBrush(
      D2D1::ColorF( D2D1::ColorF( HIGHLIGHT_COLOR, 1.0f ) ),
      &gpBrushHighlight
   );
   CHECK_RESULT( "Failed to create Direct2D Brush (Highlight)" );

   // This brush does not get used, so we will comment it out for now
// hr = gpRenderTarget->CreateSolidColorBrush(
//    D2D1::ColorF( D2D1::ColorF( BACKGROUND_COLOR, 1.0f ) ),
//    &gpBrushBackground   
// );
// CHECK_RESULT( "Failed to create Direct2D Brush (Background)" );

   return TRUE;
}


/// Cleanup the resources (in reverse order of their creation)
///
/// The IUnknown::Release() method does not have a meaningful result code
/// @see https://learn.microsoft.com/en-us/windows/win32/api/unknwn/nf-unknwn-iunknown-release
///
/// @return `true` if successful.  `false` if something went wrong.
BOOL mvcViewCleanupResources() {
   SAFE_RELEASE( gpBrushForeground );
   SAFE_RELEASE( gpBrushHighlight );
// SAFE_RELEASE( gpBrushBackground );  // It's not used, so we are commenting it out for now
   SAFE_RELEASE( gpRenderTarget );
   SAFE_RELEASE( gpD2DFactory );

   return TRUE;
}



/// Paint the main window containing the DTMF keypad 
/// 
/// @todo Modify this code so that only the specific keys and lables that
///       have changed get redrawn.  This gets rid of flicker.   
/// @see  http://www.catch22.net/tuts/win32/flicker-free-drawing#
///
/// @return `true` if successful.  `false` if there were problems.
BOOL mvcViewPaintWindow() {
   HRESULT hr;  // HRESULT result

   assert( gpRenderTarget != NULL );

   gpRenderTarget->BeginDraw() ;  
   // No return value

   /// Clear to the background color.
   /// Failing to do this makes the window smudgy.
   gpRenderTarget->Clear( D2D1::ColorF( BACKGROUND_COLOR, 1.0f ) );
   // No return value

   /// Draw the main window, generally top-down.  I'm unrolling the loops for a 
   /// little bit of an optimization
   paintColFreq( 0 );   paintColFreq( 1 );   paintColFreq( 2 );   paintColFreq( 3 );

   /// Paint each of the row lables and digits
   /// 
   /// @internal For now, and mostly for the sake of efficiency, we are not checking
   ///           the results of these drawing functions.  None of the methods these
   ///           functions call/use return any error messages, so (right now) they
   ///           always return true.
   paintRowFreq( 0 );
   paintKey(  0 );   paintKey(  1 );   paintKey(  2 );   paintKey(  3 );
   paintRowFreq( 1 );
   paintKey(  4 );   paintKey(  5 );   paintKey(  6 );   paintKey(  7 );
   paintRowFreq( 2 );
   paintKey(  8 );   paintKey(  9 );   paintKey( 10 );   paintKey( 11 );
   paintRowFreq( 3 );
   paintKey( 12 );   paintKey( 13 );   paintKey( 14 );   paintKey( 15 );

   hr = gpRenderTarget->EndDraw();
   CHECK_RESULT( "Failed to end drawing operations on the render target " );

   hasDtmfTonesChanged = false;  /// Reset the #hasDtmfTonesChanged bit

   return TRUE;
}


/// Paint a frequency label before each row
/// 
/// If a DTMF tone is detected for this frequency, then paint it using the
/// highlighted font.  Otherwise, use the normal foreground font.
/// 
/// @param index Indicates the row to paint
/// @return `true` if successful.  `false` if there were problems.
BOOL paintRowFreq( size_t index ) {
   keypad_t* pKey = &keypad[ (4 * index) + index ];  // The diaganol DTMF digits 1, 5, 9 and D
   size_t iFreq = pKey->row;

   ID2D1SolidColorBrush* pBrush;

   if ( dtmfTones[ iFreq ].detected == TRUE ) {
      pBrush = gpBrushHighlight;
   } else {
      pBrush = gpBrushForeground;
   }

   D2D1_RECT_F freqTextRect = D2D1::RectF(
      static_cast<FLOAT>( COL0 - 78 ),
      static_cast<FLOAT>( pKey->y ),
      static_cast<FLOAT>( COL0 - 32 ),
      static_cast<FLOAT>( pKey->y + BOX_HEIGHT )
   );
   // No return value (for error checking)

   // Get text length
   UINT32 cTextLength = (UINT32) wcslen( dtmfTones[ iFreq ].label );	  

   gpRenderTarget->DrawText(
      dtmfTones[ iFreq ].label, // Text to render
      cTextLength,              // Text length
      gpFreqTextFormat,         // Text format
      freqTextRect,	           // The region of the window where the text will be rendered
      pBrush                    // The brush used to draw the text
   );
   // No return value (for error checking)

   freqTextRect = D2D1::RectF(
      static_cast<FLOAT>( COL0 - 32 ),
      static_cast<FLOAT>( pKey->y - 4 ),
      static_cast<FLOAT>( COL0 - 12 ),
      static_cast<FLOAT>( pKey->y + BOX_HEIGHT - 4)
   );
   // No return value (for error checking)

   const wchar_t* wszText = L"Hz";		        // String to render
   cTextLength = (UINT32) wcslen( wszText );	  // Get text length

   gpRenderTarget->DrawText(
      wszText,                  // Text to render
      cTextLength,              // Text length
      gpLettersTextFormat,      // Text format
      freqTextRect,	           // The region of the window where the text will be rendered
      pBrush                    // The brush used to draw the text
   );
   // No return value (for error checking)

   return TRUE;
}


/// Paint a frequency label above each column
/// 
/// If a DTMF tone is detected for this frequency, then paint it using the
/// highlighted font.  Otherwise, use the normal foreground font.
/// 
/// @param index Indicates the column to paint
/// @return `true` if successful.  `false` if there were problems.
BOOL paintColFreq( size_t index ) {
   keypad_t* pKey = &keypad[ ( 4 * index ) + index ];  // The diaganol DTMF digits 1, 5, 9 and D
   size_t iFreq = pKey->column;

   ID2D1SolidColorBrush* pBrush;

   if ( dtmfTones[ iFreq ].detected == TRUE ) {
      pBrush = gpBrushHighlight;
   } else {
      pBrush = gpBrushForeground;
   }

   D2D1_RECT_F freqTextRect = D2D1::RectF(
      static_cast<FLOAT>( pKey->x - 16 ),
      static_cast<FLOAT>( ROW0 - 48 ),
      static_cast<FLOAT>( pKey->x + BOX_WIDTH - 16 ),
      static_cast<FLOAT>( ROW0 )
   );
   // No return value (for error checking)

   // Get text length
   UINT32 cTextLength = (UINT32) wcslen( dtmfTones[ iFreq ].label );	

   gpRenderTarget->DrawText(
      dtmfTones[ iFreq ].label, // Text to render
      cTextLength,              // Text length
      gpFreqTextFormat,         // Text format
      freqTextRect,	           // The region of the window where the text will be rendered
      pBrush                    // The brush used to draw the text
   );
   // No return value (for error checking)

   freqTextRect = D2D1::RectF(
      static_cast<FLOAT>( pKey->x + 47 ),
      static_cast<FLOAT>( ROW0 - 36 ),
      static_cast<FLOAT>( pKey->x + 71 ),
      static_cast<FLOAT>( ROW0 - 20 )
   );
   // No return value (for error checking)

   const wchar_t* wszText = L"Hz";		        // String to render
   cTextLength = (UINT32) wcslen( wszText );	  // Get text length

   gpRenderTarget->DrawText(
      wszText,                  // Text to render
      cTextLength,              // Text length
      gpLettersTextFormat,      // Text format
      freqTextRect,	           // The region of the window where the text will be rendered
      pBrush                    // The brush used to draw the text
   );
   // No return value (for error checking)

   return TRUE;
}


/// The detailed work to paint each key on the keypad... painting the rectangle,
/// digit and letters above the digit.
/// 
/// If a DTMF tone is detected for this combination of frequencies, then paint 
/// it using the highlighted font.  Otherwise, use the normal foreground font.
///
/// @param index Indicates the key to paint
/// @return `true` if successful.  `false` if there were problems.
BOOL paintKey( size_t index ) {
   ID2D1SolidColorBrush* pBrush;

   if ( dtmfTones[ keypad[ index ].row ].detected == TRUE && dtmfTones[ keypad[ index ].column ].detected == TRUE ) {
      pBrush = gpBrushHighlight;
   } else {
      pBrush = gpBrushForeground;
   }

   gpRenderTarget->DrawRoundedRectangle(
      D2D1::RoundedRect(
         D2D1::RectF( keypad[index].x, keypad[index].y, keypad[index].x + BOX_WIDTH, keypad[index].y + BOX_HEIGHT ),
         8.0f,
         8.0f ),  // const D2D1_ROUNDED_RECT
      pBrush,     // Brush
      2.f ) ;     // Stroke width
      // No return value (for error checking)


   D2D1_RECT_F digitTextRect = D2D1::RectF(
      static_cast<FLOAT>( keypad[index].x ),
      static_cast<FLOAT>( keypad[index].y + 24 ),
      static_cast<FLOAT>( keypad[index].x + BOX_WIDTH ),
      static_cast<FLOAT>( keypad[index].y + BOX_HEIGHT - 6 )
   );
   // No return value (for error checking)

   // Get text length
   UINT32 cTextLength = (UINT32) wcslen( keypad[index].digit);	  

   gpRenderTarget->DrawText(
      keypad[index].digit, // Text to render
      cTextLength,         // Text length
      gpDigitTextFormat,   // Text format
      digitTextRect,	      // The region of the window where the text will be rendered
      pBrush               // The brush used to draw the text
   );
   // No return value (for error checking)

   // Draw the letters above the digits
   cTextLength = (UINT32) wcslen( keypad[ index ].letters );	  // Get text length

   if ( cTextLength > 0 ) {
      D2D1_RECT_F lettersTextRect = D2D1::RectF(
         static_cast<FLOAT>( keypad[ index ].x ),
         static_cast<FLOAT>( keypad[ index ].y + 6 ),
         static_cast<FLOAT>( keypad[ index ].x + BOX_WIDTH ),
         static_cast<FLOAT>( keypad[ index ].y + 6 + 16 )
      );
      // No return value (for error checking)


      gpRenderTarget->DrawText(
         keypad[ index ].letters,  // Text to render
         cTextLength,              // Text length
         gpLettersTextFormat,      // Text format
         lettersTextRect,          // The region of the window where the text will be rendered
         pBrush                    // The brush used to draw the text
      );
      // No return value (for error checking)
   }

   return TRUE;
}
