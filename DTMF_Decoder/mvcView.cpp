///////////////////////////////////////////////////////////////////////////////
//          University of Hawaii, College of Engineering
//          DTMF_Decoder - EE 469 - Fall 2022
//
//  A Windows Desktop C program that decodes DTMF tones
//
/// The implementation of the GDI and Direct2D paint commands as the view
/// component of this model-view-controller application.
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
/// @file    mvcView.cpp
/// @version 2.0
///
/// @author  Mark Nelson <marknels@hawaii.edu>
/// @date    10_Oct_2022
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
static ID2D1HwndRenderTarget* spRenderTarget      = NULL;  ///< Render target
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
   LONG   x;          ///< The upper-left corner of the digit's box
   LONG   y;          ///< The upper-left corner of the digit's box
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


/// Invalidate just the row (not the whole screen)
///
/// @param row Index of the row... 0 through 3.
///
/// @return `true` if successful.  `false` if there was a problem.
BOOL mvcInvalidateRow( _In_ const size_t row ) {
   _ASSERTE( row <= 3 );
   _ASSERTE( ghMainWindow != NULL );

   BOOL br;            // BOOL result
   RECT rectToRedraw;  // The rectangle to redraw

   rectToRedraw.left = 0;
   rectToRedraw.right = giWindowWidth;

   switch ( row ) {
      case 0:
         rectToRedraw.top    = ROW0;
         rectToRedraw.bottom = ROW0 + BOX_HEIGHT;
         break;
      case 1:
         rectToRedraw.top    = ROW1;
         rectToRedraw.bottom = ROW1 + BOX_HEIGHT;
         break;
      case 2:
         rectToRedraw.top    = ROW2;
         rectToRedraw.bottom = ROW2 + BOX_HEIGHT;
         break;
      case 3:
         rectToRedraw.top    = ROW3;
         rectToRedraw.bottom = ROW3 + BOX_HEIGHT;
         break;
   }

   br = InvalidateRect( ghMainWindow, &rectToRedraw, FALSE );
   CHECK_BR( "Failed to invalidate rectangle" );

   return TRUE;
}


/// Invalidate the column (not the whole screen)
///
/// @param column Index of the column... 0 through 3.
///
/// @return `true` if successful.  `false` if there was a problem.
BOOL mvcInvalidateColumn( _In_ const size_t column ) {
   _ASSERTE( column <= 3 );
   _ASSERTE( ghMainWindow != NULL );

   BOOL br;            // BOOL result
   RECT rectToRedraw;  // The rectangle to redraw

   rectToRedraw.top = 0;
   rectToRedraw.bottom = giWindowHeight;

   switch ( column ) {
      case 0:
         rectToRedraw.left  = COL0 - 16;
         rectToRedraw.right = COL0 + 71;
         break;
      case 1:
         rectToRedraw.left  = COL1 - 16;
         rectToRedraw.right = COL1 + 71;
         break;
      case 2:
         rectToRedraw.left  = COL2 - 16;
         rectToRedraw.right = COL2 + 71;
         break;
      case 3:
         rectToRedraw.left  = COL3 - 16;
         rectToRedraw.right = COL3 + 71;
         break;
   }

   br = InvalidateRect( ghMainWindow, &rectToRedraw, FALSE );
   CHECK_BR( "Failed to invalidate rectangle" );

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


   _ASSERTE( spBrushBackground == NULL );
   hr = spRenderTarget->CreateSolidColorBrush(
      D2D1::ColorF( D2D1::ColorF( BACKGROUND_COLOR, 1.0f ) ),
      &spBrushBackground
   );
   CHECK_HR( "Failed to create Direct2D Brush (Background)" );
   _ASSERTE( spBrushBackground != NULL );

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
   SAFE_RELEASE( spBrushBackground );
   SAFE_RELEASE( spRenderTarget );
   SAFE_RELEASE( spD2DFactory );

   return TRUE;
}


/// Populate a D2D1_RECT_F rectangle and ensure that the new rectangle needs to
/// be updated by checking pUpdateRect.
///
/// Many thanks to Silent Matt for validating an optimized algorithm for
/// rectangle overlap detection.
/// @see https://stackoverflow.com/questions/306316/determine-if-two-rectangles-overlap-each-other
/// @see https://silentmatt.com/rectangle-intersection/
///
/// @param pRect_F      Pointer to the floating-point rectangle structure that
///                     will be populated **if** the rectangle needs updating.
///                     If the rectangle does not need updating, this structure
///                     does not change.
/// @param pUpdateRect  The rectangle that needs to be refreshed (from WM_PAINT)
/// @param left         The rectangle that's being made
/// @param top          The rectangle that's being made
/// @param right        The rectangle that's being made
/// @param bottom       The rectangle that's being made
/// @return `TRUE` if the rectangle that's being made has any overlap with
///         pUpdateRect.  `FALSE` if the rectangle that's being made is outside
///         of pUpdateRect (which probably means it doesn't need to be redrawn).
static inline BOOL makeFloatRect(
   _Out_ D2D1_RECT_F* pRect_F,
   _In_  const RECT*  pUpdateRect,
   _In_  const LONG   left,
   _In_  const LONG   top,
   _In_  const LONG   right,
   _In_  const LONG   bottom ) {

   if ( pUpdateRect->left >= right )  /// Fail fast if the rectangle is outside the update region
      return FALSE;
   if ( pUpdateRect->right <= left )
      return FALSE;
   if ( pUpdateRect->top >= bottom )
      return FALSE;
   if ( pUpdateRect->bottom <= top )
      return FALSE;

   // At this point, the rectangle is inside the update region
   pRect_F->left = static_cast<FLOAT>( left );
   pRect_F->top = static_cast<FLOAT>( top );
   pRect_F->right = static_cast<FLOAT>( right );
   pRect_F->bottom = static_cast<FLOAT>( bottom );

   // LOG_TRACE( "Drawing rectangle:  left=%ld  top=%ld  right=%ld  bottom=%ld", left, top, right, bottom );

   return TRUE;
}


/// Paint a frequency label before each row
///
/// If a DTMF tone is detected for this frequency, then paint it using the
/// highlighted font.  Otherwise, use the normal foreground font.
///
/// This function won't update content that's outside pUpdateRect
///
/// @param pUpdateRect The rectangle to update (from WM_PAINT)
static inline void paintRowFreqs( _In_ const RECT* pUpdateRect ) {
   _ASSERTE( spRenderTarget      != NULL );
   _ASSERTE( spBrushHighlight    != NULL );
   _ASSERTE( spBrushForeground   != NULL );
   _ASSERTE( spLettersTextFormat != NULL );
   _ASSERTE( spFreqTextFormat    != NULL );
   _ASSERTE( pUpdateRect         != NULL );

   for ( size_t index = 0 ; index < 4 ; index++ ) {
      keypad_t* pKey = &keypad[ ( 4 * index ) + index ];  // The diaganol DTMF digits 1, 5, 9 and D
      size_t iFreq = pKey->row;

      ID2D1SolidColorBrush* pBrush;

      if ( gDtmfTones[ iFreq ].detected == TRUE ) {
         pBrush = spBrushHighlight;
      } else {
         pBrush = spBrushForeground;
      }

      D2D1_RECT_F freqTextRect;
      UINT32      cTextLength;

      if ( makeFloatRect( &freqTextRect, pUpdateRect, COL0 - 78, pKey->y, COL0 - 32, pKey->y + BOX_HEIGHT ) ) {
         // Get text length
         cTextLength = (UINT32) wcslen( gDtmfTones[ iFreq ].label );

         spRenderTarget->DrawText(
            gDtmfTones[ iFreq ].label, // Text to render
            cTextLength,               // Text length
            spFreqTextFormat,          // Text format
            freqTextRect,	            // The region of the window where the text will be rendered
            pBrush                     // The brush used to draw the text
         );                            // No return value to check for erors
      }

      if ( makeFloatRect( &freqTextRect, pUpdateRect, COL0 - 32, pKey->y - 4, COL0 - 12, pKey->y + BOX_HEIGHT - 4 ) ) {
         const wchar_t* wszText = L"Hz";		        // String to render
         cTextLength = (UINT32) wcslen( wszText );	  // Get text length

         spRenderTarget->DrawText(
            wszText,              // Text to render
            cTextLength,          // Text length
            spLettersTextFormat,  // Text format
            freqTextRect,	       // The region of the window where the text will be rendered
            pBrush                // The brush used to draw the text
         );                       // No return value to check for erors
      }
   }
}


/// Paint a frequency label above each column
///
/// If a DTMF tone is detected for this frequency, then paint it using the
/// highlighted font.  Otherwise, use the normal foreground font.
///
/// This function won't update content that's outside pUpdateRect
///
/// @param pUpdateRect The rectangle to update (from WM_PAINT)
static inline void paintColFreqs( _In_ const RECT* pUpdateRect ) {
   _ASSERTE( spRenderTarget      != NULL );
   _ASSERTE( spBrushHighlight    != NULL );
   _ASSERTE( spBrushForeground   != NULL );
   _ASSERTE( spLettersTextFormat != NULL );
   _ASSERTE( spFreqTextFormat    != NULL );
   _ASSERTE( pUpdateRect         != NULL );

   for ( size_t index = 0 ; index < 4 ; index++ ) {
      keypad_t* pKey = &keypad[ ( 4 * index ) + index ];  // The diaganol DTMF digits 1, 5, 9 and D
      size_t iFreq = pKey->column;

      ID2D1SolidColorBrush* pBrush;

      if ( gDtmfTones[ iFreq ].detected == TRUE ) {
         pBrush = spBrushHighlight;
      } else {
         pBrush = spBrushForeground;
      }

      D2D1_RECT_F freqTextRect;
      UINT32      cTextLength;

      if ( makeFloatRect( &freqTextRect, pUpdateRect, pKey->x - 16, ROW0 - 48, pKey->x + BOX_WIDTH - 16, ROW0 ) ) {
         // Get text length
         cTextLength = (UINT32) wcslen( gDtmfTones[ iFreq ].label );

         spRenderTarget->DrawText(
            gDtmfTones[ iFreq ].label, // Text to render
            cTextLength,               // Text length
            spFreqTextFormat,          // Text format
            freqTextRect,	            // The region of the window where the text will be rendered
            pBrush                     // The brush used to draw the text
         );                            // No return value to check for erors
      }

      if ( makeFloatRect( &freqTextRect, pUpdateRect, pKey->x + 47, ROW0 - 36, pKey->x + 71, ROW0 - 20 ) ) {
         const wchar_t* wszText = L"Hz";		        // String to render
         cTextLength = (UINT32) wcslen( wszText );	  // Get text length

         spRenderTarget->DrawText(
            wszText,              // Text to render
            cTextLength,          // Text length
            spLettersTextFormat,  // Text format
            freqTextRect,	       // The region of the window where the text will be rendered
            pBrush                // The brush used to draw the text
         );                       // No return value to check for erors
      }
   }
}


/// The detailed work to paint each key on the keypad... painting the rectangle,
/// digit and letters above the digit.
///
/// If a DTMF tone is detected for this combination of frequencies, then paint
/// it using the highlighted font.  Otherwise, use the normal foreground font.
///
/// This function won't update content that's outside pUpdateRect
///
/// @param pUpdateRect The rectangle to update (from WM_PAINT)
static inline void paintKeys( _In_ const RECT* pUpdateRect ) {
   _ASSERTE( spRenderTarget      != NULL );
   _ASSERTE( spBrushHighlight    != NULL );
   _ASSERTE( spBrushForeground   != NULL );
   _ASSERTE( spDigitTextFormat   != NULL );
   _ASSERTE( spLettersTextFormat != NULL );
   _ASSERTE( pUpdateRect         != NULL );

   for ( size_t index = 0 ; index < 16 ; index++ ) {
      ID2D1SolidColorBrush* pBrush;

      if ( gDtmfTones[ keypad[ index ].row ].detected == TRUE && gDtmfTones[ keypad[ index ].column ].detected == TRUE ) {
         pBrush = spBrushHighlight;
      } else {
         pBrush = spBrushForeground;
      }

      D2D1_RECT_F drawingRect;
      UINT32 cTextLength;

      /// Draw the box around the keypad
      if ( makeFloatRect( &drawingRect, pUpdateRect, keypad[ index ].x, keypad[ index ].y, keypad[ index ].x + BOX_WIDTH, keypad[ index ].y + BOX_HEIGHT ) ) {
         spRenderTarget->DrawRoundedRectangle(
            D2D1::RoundedRect( drawingRect, 8.0f, 8.0f ),  // const D2D1_ROUNDED_RECT
            pBrush,     // Brush
            2.f ) ;     // Stroke width
            // No return value to check for erors
      }


      /// Draw the large digit (label)
      if ( makeFloatRect( &drawingRect, pUpdateRect, keypad[ index ].x, keypad[ index ].y + 24, keypad[ index ].x + BOX_WIDTH, keypad[ index ].y + BOX_HEIGHT -6 ) ) {
         // Get text length
         cTextLength = (UINT32) wcslen( keypad[ index ].digit );

         spRenderTarget->DrawText(
            keypad[ index ].digit, // Text to render
            cTextLength,           // Text length
            spDigitTextFormat,     // Text format
            drawingRect,	        // The region of the window where the text will be rendered
            pBrush                 // The brush used to draw the text
         );                        // No return value to check for erors
      }


      /// Draw the letters above the digits
      if ( makeFloatRect( &drawingRect, pUpdateRect, keypad[ index ].x, keypad[ index ].y + 6, keypad[ index ].x + BOX_WIDTH, keypad[ index ].y + 6 + 16 ) ) {
         cTextLength = (UINT32) wcslen( keypad[ index ].letters );	  // Get text length

         if ( cTextLength > 0 ) {
            spRenderTarget->DrawText(
               keypad[ index ].letters,  // Text to render
               cTextLength,              // Text length
               spLettersTextFormat,      // Text format
               drawingRect,              // The region of the window where the text will be rendered
               pBrush                    // The brush used to draw the text
            );                           // No return value to check for erors
         }
      }
   }
}


/// Paint the main window containing the DTMF keypad
///
/// @see  http://www.catch22.net/tuts/win32/flicker-free-drawing#
///
/// @param pUpdateRect The rectangle to update (from WM_PAINT)
/// @return `true` if successful.  `false` if there were problems.
BOOL mvcViewPaintWindow( _In_ const RECT* pUpdateRect ) {
   HRESULT hr;  // HRESULT result

   _ASSERTE( spRenderTarget != NULL );
   _ASSERTE( pUpdateRect != NULL );

   // LOG_TRACE( "Update region:  (%d, %d) to (%d, %d)", pUpdateRect->left, pUpdateRect->top, pUpdateRect->right, pUpdateRect->bottom );

   spRenderTarget->BeginDraw();          // No return value to check for errors

   /// Paint the background color into the update region.
   _ASSERTE( spBrushBackground != NULL );

   D2D1_RECT_F updateRect_F = D2D1::RectF(
      static_cast<FLOAT>( pUpdateRect->left ),
      static_cast<FLOAT>( pUpdateRect->top ),
      static_cast<FLOAT>( pUpdateRect->right ),
      static_cast<FLOAT>( pUpdateRect->bottom )
   );                                     // No return value to check for erors

   spRenderTarget->FillRectangle(
      updateRect_F,         // The rectangle to fill
      spBrushBackground     // The background brush
   );                                     // No return value to check for erors

   /// Draw the main window... starting with the column and row labels
   ///
   /// @internal For now, and mostly for the sake of efficiency, we are not checking
   ///           the results of these drawing functions.  None of the methods these
   ///           functions call/use return any error messages, so (right now)
   ///           they return `void`.
   paintColFreqs( pUpdateRect );
   paintRowFreqs( pUpdateRect );
   paintKeys( pUpdateRect );

   hr = spRenderTarget->EndDraw();
   CHECK_HR( "Failed to end drawing operations on the render target " );

   return TRUE;
}
