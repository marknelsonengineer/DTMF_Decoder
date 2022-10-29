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
/// @see https://learn.microsoft.com/en-us/windows/win32/api/d2d1/nf-d2d1-d2d1createfactory-r1
/// @see https://learn.microsoft.com/en-us/windows/win32/api/dwrite/nf-dwrite-dwritecreatefactory
/// @see https://learn.microsoft.com/en-us/windows/win32/api/dwrite/nf-dwrite-idwritefactory-createtextformat
/// @see https://learn.microsoft.com/en-us/windows/win32/api/dwrite/nf-dwrite-idwritetextformat-setwordwrapping
/// @see https://learn.microsoft.com/en-us/windows/win32/api/dwrite/nf-dwrite-idwritetextformat-settextalignment
/// @see https://learn.microsoft.com/en-us/windows/win32/api/dwrite/nf-dwrite-idwritetextformat-setparagraphalignment
/// @see https://learn.microsoft.com/en-us/windows/win32/api/d2d1/nf-d2d1-id2d1factory-createhwndrendertarget(constd2d1_render_target_properties__constd2d1_hwnd_render_target_properties__id2d1hwndrendertarget)
/// @see https://learn.microsoft.com/en-us/windows/win32/api/d2d1/nf-d2d1-id2d1rendertarget-createsolidcolorbrush(constd2d1_color_f__id2d1solidcolorbrush)
/// @see https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-invalidaterect
/// @see https://learn.microsoft.com/en-us/windows/win32/api/unknwn/nf-unknwn-iunknown-release
/// @see https://learn.microsoft.com/en-us/windows/win32/api/d2d1/nf-d2d1-id2d1rendertarget-begindraw
/// @see https://learn.microsoft.com/en-us/windows/win32/api/d2d1/nf-d2d1-id2d1rendertarget-enddraw
/// @see https://learn.microsoft.com/en-us/windows/win32/direct2d/id2d1rendertarget-clear
/// @see https://learn.microsoft.com/en-us/windows/win32/api/d2d1/nf-d2d1-id2d1rendertarget-drawtext(constwchar_uint32_idwritetextformat_constd2d1_rect_f__id2d1brush_d2d1_draw_text_options_dwrite_measuring_mode)
/// @see https://learn.microsoft.com/en-us/windows/win32/api/d2d1helper/nf-d2d1helper-rectf
/// @see https://learn.microsoft.com/en-us/dotnet/api/system.windows.media.drawingcontext.drawroundedrectangle?view=windowsdesktop-6.0
///
/// @see https://en.cppreference.com/w/c/string/wide/wcslen
///
/// @author Mark Nelson <marknels@hawaii.edu>
/// @date   10_Oct_2022
///////////////////////////////////////////////////////////////////////////////

#include "framework.h"    // Standard system include files
#include <d2d1.h>         // For Direct2D (drawing)
#include <dwrite.h>       // For DirectWrite (fonts and text)

#include "mvcView.h"      // For yo bad self
#include "mvcModel.h"     // For viewing the state of the machine

#pragma comment(lib, "d2d1")    // Link the Diect2D library (for drawing)
#pragma comment(lib, "Dwrite")  // Link the DirectWrite library (for fonts and text)


// Global Variables (private to this source file)
static ID2D1Factory*          spD2DFactory        = NULL;  ///< The Direct2D Factory
static ID2D1HwndRenderTarget* spRenderTarget      = NULL;	 ///< Render target
static ID2D1SolidColorBrush*  spBrushForeground   = NULL;  ///< A light blue brush for the foreground
static ID2D1SolidColorBrush*  spBrushHighlight    = NULL;  ///< A lighter blue brush for the highlight
static ID2D1SolidColorBrush*  spBrushBackground   = NULL;  ///< A dark blue brush for the background (not used right now)
static IDWriteFactory*        spDWriteFactory     = NULL;  ///< A DirectWrite factory object
static IDWriteTextFormat*     spDigitTextFormat   = NULL;  ///< The font for the digits
static IDWriteTextFormat*     spLettersTextFormat = NULL;  ///< The font for the letters above the digits (and the `Hz` units)
static IDWriteTextFormat*     spFreqTextFormat    = NULL;  ///< The font for the frequency

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

const int giWindowWidth  = COL0 + ( BOX_WIDTH  * 4 ) + ( GAP_WIDTH  * 3 ) + BOX_WIDTH;
const int giWindowHeight = ROW0 + ( BOX_HEIGHT * 4 ) + ( GAP_HEIGHT * 3 ) + BOX_HEIGHT + 50;  // The title is 25px and the menu is 25px

/// Holds the location and display data for each key (button) on the keypad
typedef struct {
   WCHAR  digit[2];   ///< The digit to print
   size_t row;        ///< Row index into gDtmfTones
   size_t column;     ///< Column index into gDtmfTones
   WCHAR  letters[5]; ///< The letters above the digit
   float  x;          ///< The upper-left corner of the digit's box
   float  y;          ///< The upper-left corner of the digit's box
} keypad_t;


/// An array of information (location and display data) for each key (button)
/// on the keypad
static keypad_t keypad[ 16 ] = {
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
static BOOL paintKey( _In_ size_t index );
static BOOL paintRowFreq( _In_ size_t index );
static BOOL paintColFreq( _In_ size_t index );


/// Repaint the main window (keypad) -- probably because the state
/// of one of the buttons has changed
///
/// @return `true` if successful.  `false` if there was a problem.
BOOL mvcViewRefreshWindow() {
   BOOL    br;  // BOOL result

   _ASSERTE( ghMainWindow != NULL );

   br = InvalidateRect( ghMainWindow, NULL, FALSE );
   CHECK_BR( "Failed to invalidate rectangle" );

   br = UpdateWindow( ghMainWindow );
   CHECK_BR( "Failed to update window" );

   return TRUE;
}


/// Initialize all of the resources needed to draw the main window
///
/// @return `true` if successful.  `false` if there was a problem.
BOOL mvcViewInitResources() {
   HRESULT hr;  // HRESULT result
   BOOL    br;  // BOOL result

   _ASSERTE( ghMainWindow != NULL );

   /// Initialize Direct2D
   _ASSERTE( spD2DFactory == NULL );
   hr = D2D1CreateFactory( D2D1_FACTORY_TYPE_MULTI_THREADED, &spD2DFactory );
   CHECK_HR( "Failed to create Direct2D Factory" );
   _ASSERTE( spD2DFactory != NULL );

   /// Initialize DirectWrite
   _ASSERTE( spDWriteFactory == NULL );
   hr = DWriteCreateFactory(
      DWRITE_FACTORY_TYPE_SHARED,
      __uuidof( IDWriteFactory ),
      reinterpret_cast<IUnknown**>( &spDWriteFactory )
   );
   CHECK_HR( "Failed to create DirectWrite Factory" );
   _ASSERTE( spDWriteFactory != NULL );

   /// Create the font for the digits
   _ASSERTE( spDigitTextFormat == NULL );
   hr = spDWriteFactory->CreateTextFormat(
      L"Segoe UI",                  // Font family name
      NULL,                         // Font collection(NULL sets it to the system font collection)
      DWRITE_FONT_WEIGHT_MEDIUM,    // Weight
      DWRITE_FONT_STYLE_NORMAL,     // Style
      DWRITE_FONT_STRETCH_NORMAL,   // Stretch
      36.0f,                        // Size
      L"en-us",                     // Local
      &spDigitTextFormat            // Pointer to recieve the created object
   );
   CHECK_HR( "Failed to create a font resource (digit text format)" );
   _ASSERTE( spDigitTextFormat != NULL );

   hr = spDigitTextFormat->SetWordWrapping( DWRITE_WORD_WRAPPING_WRAP );
   CHECK_HR( "Failed to set word wrap mode (digit text format)" );

   hr = spDigitTextFormat->SetTextAlignment( DWRITE_TEXT_ALIGNMENT_CENTER );  // Horizontal alignment
   CHECK_HR( "Failed to set text alignment (digit text format)" );

   hr = spDigitTextFormat->SetParagraphAlignment( DWRITE_PARAGRAPH_ALIGNMENT_CENTER );  // Vertical alignment
   CHECK_HR( "Failed to set paragraph alighment (digit text format)" );

   /// Create the font for the letters
   _ASSERTE( spLettersTextFormat == NULL );
   hr = spDWriteFactory->CreateTextFormat(
      L"Segoe UI",                  // Font family name
      NULL,                         // Font collection(NULL sets it to the system font collection)
      DWRITE_FONT_WEIGHT_REGULAR,   // Weight
      DWRITE_FONT_STYLE_NORMAL,     // Style
      DWRITE_FONT_STRETCH_NORMAL,   // Stretch
      16.0f,                        // Size
      L"en-us",                     // Local
      &spLettersTextFormat          // Pointer to recieve the created object
   );
   CHECK_HR( "Failed to create a font resource (letters text format)" );
   _ASSERTE( spLettersTextFormat != NULL );

   hr = spLettersTextFormat->SetWordWrapping( DWRITE_WORD_WRAPPING_WRAP );
   CHECK_HR( "Failed to set word wrap mode (letters text format)" );

   hr = spLettersTextFormat->SetTextAlignment( DWRITE_TEXT_ALIGNMENT_CENTER );  // Horizontal alignment
   CHECK_HR( "Failed to set text alignment (letters text format)" );

   hr = spLettersTextFormat->SetParagraphAlignment( DWRITE_PARAGRAPH_ALIGNMENT_CENTER );  // Vertical alignment
   CHECK_HR( "Failed to set paragraph alighment (letters text format)" );

   /// Create the font for the frequency labels
   _ASSERTE( spFreqTextFormat == NULL );
   hr = spDWriteFactory->CreateTextFormat(
      L"Segoe UI",                  // Font family name
      NULL,                         // Font collection(NULL sets it to the system font collection)
      DWRITE_FONT_WEIGHT_REGULAR,   // Weight
      DWRITE_FONT_STYLE_NORMAL,     // Style
      DWRITE_FONT_STRETCH_NORMAL,   // Stretch
      24.0f,                        // Size
      L"en-us",                     // Local
      &spFreqTextFormat             // Pointer to recieve the created object
   );
   CHECK_HR( "Failed to create a font resource (frequency text format)" );
   _ASSERTE( spFreqTextFormat != NULL );

   hr = spFreqTextFormat->SetWordWrapping( DWRITE_WORD_WRAPPING_WRAP );
   CHECK_HR( "Failed to set word wrap mode (frequency text format)" );

   hr = spFreqTextFormat->SetTextAlignment( DWRITE_TEXT_ALIGNMENT_TRAILING );  // Horizontal alignment
   CHECK_HR( "Failed to set text alignment (frequency text format)" );

   hr = spFreqTextFormat->SetParagraphAlignment( DWRITE_PARAGRAPH_ALIGNMENT_CENTER );  // Vertical alignment
   CHECK_HR( "Failed to set paragraph alighment (frequency text format)" );

   /// Create the Direct2D render target
   RECT clientRectangle ;
   br = GetClientRect( ghMainWindow, &clientRectangle );
   CHECK_BR( "Failed to get the window size" );

   _ASSERTE( spRenderTarget == NULL );
   hr = spD2DFactory->CreateHwndRenderTarget(
      D2D1::RenderTargetProperties(),
      D2D1::HwndRenderTargetProperties( ghMainWindow, D2D1::SizeU( clientRectangle.right - clientRectangle.left, clientRectangle.bottom - clientRectangle.top ) ),
      &spRenderTarget
   );
   CHECK_HR( "Failed to create Direct2D Render Target" );
   _ASSERTE( spRenderTarget != NULL );

   /// Create the colors (brushes) for the foreground, highlight and background
   _ASSERTE( spBrushForeground == NULL );
   hr = spRenderTarget->CreateSolidColorBrush(
      D2D1::ColorF( D2D1::ColorF( FOREGROUND_COLOR, 1.0f ) ),
      &spBrushForeground
   );
   CHECK_HR( "Failed to create Direct2D Brush (Foreground)" );
   _ASSERTE( spBrushForeground != NULL );

   _ASSERTE( spBrushHighlight == NULL );
   hr = spRenderTarget->CreateSolidColorBrush(
      D2D1::ColorF( D2D1::ColorF( HIGHLIGHT_COLOR, 1.0f ) ),
      &spBrushHighlight
   );
   CHECK_HR( "Failed to create Direct2D Brush (Highlight)" );
   _ASSERTE( spBrushHighlight != NULL );

   // This brush does not get used, so we will comment it out for now
// _ASSERTE( spBrushBackground == NULL );
// hr = spRenderTarget->CreateSolidColorBrush(
//    D2D1::ColorF( D2D1::ColorF( BACKGROUND_COLOR, 1.0f ) ),
//    &spBrushBackground
// );
// CHECK_HR( "Failed to create Direct2D Brush (Background)" );
// _ASSERTE( spBrushBackground != NULL );

   return TRUE;
}


/// Cleanup the resources (in reverse order of their creation)
///
/// The IUnknown::Release() method does not have a meaningful result code
/// @see https://learn.microsoft.com/en-us/windows/win32/api/unknwn/nf-unknwn-iunknown-release
///
/// @return `true` if successful.  `false` if something went wrong.
BOOL mvcViewCleanupResources() {
   SAFE_RELEASE( spBrushForeground );
   SAFE_RELEASE( spBrushHighlight );
// SAFE_RELEASE( spBrushBackground );  // It's not used, so we are commenting it out for now
   SAFE_RELEASE( spRenderTarget );
   SAFE_RELEASE( spD2DFactory );

   return TRUE;
}



/// Paint the main window containing the DTMF keypad
///
/// @todo Modify this code so that only the specific keys and lables that
///       have changed get redrawn.  This gets rid of flicker.  Issue #8
/// @see  http://www.catch22.net/tuts/win32/flicker-free-drawing#
///
/// @return `true` if successful.  `false` if there were problems.
BOOL mvcViewPaintWindow() {
   HRESULT hr;  // HRESULT result

   _ASSERTE( spRenderTarget != NULL );

   spRenderTarget->BeginDraw() ;
   // No return value

   /// Clear to the background color.
   /// Failing to do this makes the window smudgy.
   spRenderTarget->Clear( D2D1::ColorF( BACKGROUND_COLOR, 1.0f ) );
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

   hr = spRenderTarget->EndDraw();
   CHECK_HR( "Failed to end drawing operations on the render target " );

   gbHasDtmfTonesChanged = false;  /// Reset the #gbHasDtmfTonesChanged bit

   return TRUE;
}


/// Paint a frequency label before each row
///
/// If a DTMF tone is detected for this frequency, then paint it using the
/// highlighted font.  Otherwise, use the normal foreground font.
///
/// @param index Indicates the row to paint
/// @return `true` if successful.  `false` if there were problems.
BOOL paintRowFreq( _In_ size_t index ) {
   _ASSERTE( spRenderTarget      != NULL );
   _ASSERTE( spBrushHighlight    != NULL );
   _ASSERTE( spBrushForeground   != NULL );
   _ASSERTE( spLettersTextFormat != NULL );
   _ASSERTE( spFreqTextFormat    != NULL );

   keypad_t* pKey = &keypad[ (4 * index) + index ];  // The diaganol DTMF digits 1, 5, 9 and D
   size_t iFreq = pKey->row;

   ID2D1SolidColorBrush* pBrush;

   if ( gDtmfTones[ iFreq ].detected == TRUE ) {
      pBrush = spBrushHighlight;
   } else {
      pBrush = spBrushForeground;
   }

   D2D1_RECT_F freqTextRect = D2D1::RectF(
      static_cast<FLOAT>( COL0 - 78 ),
      static_cast<FLOAT>( pKey->y ),
      static_cast<FLOAT>( COL0 - 32 ),
      static_cast<FLOAT>( pKey->y + BOX_HEIGHT )
   );
   // No return value (for error checking)

   // Get text length
   UINT32 cTextLength = (UINT32) wcslen( gDtmfTones[ iFreq ].label );

   spRenderTarget->DrawText(
      gDtmfTones[ iFreq ].label, // Text to render
      cTextLength,              // Text length
      spFreqTextFormat,         // Text format
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

   spRenderTarget->DrawText(
      wszText,                  // Text to render
      cTextLength,              // Text length
      spLettersTextFormat,      // Text format
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
BOOL paintColFreq( _In_ size_t index ) {
   _ASSERTE( spRenderTarget      != NULL );
   _ASSERTE( spBrushHighlight    != NULL );
   _ASSERTE( spBrushForeground   != NULL );
   _ASSERTE( spLettersTextFormat != NULL );
   _ASSERTE( spFreqTextFormat    != NULL );

   keypad_t* pKey = &keypad[ ( 4 * index ) + index ];  // The diaganol DTMF digits 1, 5, 9 and D
   size_t iFreq = pKey->column;

   ID2D1SolidColorBrush* pBrush;

   if ( gDtmfTones[ iFreq ].detected == TRUE ) {
      pBrush = spBrushHighlight;
   } else {
      pBrush = spBrushForeground;
   }

   D2D1_RECT_F freqTextRect = D2D1::RectF(
      static_cast<FLOAT>( pKey->x - 16 ),
      static_cast<FLOAT>( ROW0 - 48 ),
      static_cast<FLOAT>( pKey->x + BOX_WIDTH - 16 ),
      static_cast<FLOAT>( ROW0 )
   );
   // No return value (for error checking)

   // Get text length
   UINT32 cTextLength = (UINT32) wcslen( gDtmfTones[ iFreq ].label );

   spRenderTarget->DrawText(
      gDtmfTones[ iFreq ].label, // Text to render
      cTextLength,              // Text length
      spFreqTextFormat,         // Text format
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

   spRenderTarget->DrawText(
      wszText,                  // Text to render
      cTextLength,              // Text length
      spLettersTextFormat,      // Text format
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
BOOL paintKey( _In_ size_t index ) {
   _ASSERTE( spRenderTarget      != NULL );
   _ASSERTE( spBrushHighlight    != NULL );
   _ASSERTE( spBrushForeground   != NULL );
   _ASSERTE( spDigitTextFormat   != NULL );
   _ASSERTE( spLettersTextFormat != NULL );

   ID2D1SolidColorBrush* pBrush;

   if ( gDtmfTones[ keypad[ index ].row ].detected == TRUE && gDtmfTones[ keypad[ index ].column ].detected == TRUE ) {
      pBrush = spBrushHighlight;
   } else {
      pBrush = spBrushForeground;
   }

   spRenderTarget->DrawRoundedRectangle(
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

   spRenderTarget->DrawText(
      keypad[index].digit, // Text to render
      cTextLength,         // Text length
      spDigitTextFormat,   // Text format
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


      spRenderTarget->DrawText(
         keypad[ index ].letters,  // Text to render
         cTextLength,              // Text length
         spLettersTextFormat,      // Text format
         lettersTextRect,          // The region of the window where the text will be rendered
         pBrush                    // The brush used to draw the text
      );
      // No return value (for error checking)
   }

   return TRUE;
}
