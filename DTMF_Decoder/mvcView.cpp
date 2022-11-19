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
/// ## GDI & Direct2D API
/// | API                                         | Link                                                                                                                                                                                                        |
/// |---------------------------------------------|-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
/// | `D2D1CreateFactory`                         | https://learn.microsoft.com/en-us/windows/win32/api/d2d1/nf-d2d1-d2d1createfactory-r1                                                                                                                       |
/// | `DWriteCreateFactory`                       | https://learn.microsoft.com/en-us/windows/win32/api/dwrite/nf-dwrite-dwritecreatefactory                                                                                                                    |
/// | `ID2D1RenderTarget::CreateHwndRenderTarget` | https://learn.microsoft.com/en-us/windows/win32/api/d2d1/nf-d2d1-id2d1factory-createhwndrendertarget(constd2d1_render_target_properties__constd2d1_hwnd_render_target_properties__id2d1hwndrendertarget)    |
/// | `ID2D1RenderTarget::BeginDraw`              | https://learn.microsoft.com/en-us/windows/win32/api/d2d1/nf-d2d1-id2d1rendertarget-begindraw                                                                                                                |
/// | `D2D1::RectF`                               | https://learn.microsoft.com/en-us/windows/win32/api/d2d1helper/nf-d2d1helper-rectf                                                                                                                          |
/// | `ID2D1RenderTarget::Clear`                  | https://learn.microsoft.com/en-us/windows/win32/direct2d/id2d1rendertarget-clear                                                                                                                            |
/// | `ID2D1RenderTarget::CreateSolidColorBrush`  | https://learn.microsoft.com/en-us/windows/win32/api/d2d1/nf-d2d1-id2d1rendertarget-createsolidcolorbrush(constd2d1_color_f__id2d1solidcolorbrush)                                                           |
/// | `IDWriteFactory::CreateTextFormat`          | https://learn.microsoft.com/en-us/windows/win32/api/dwrite/nf-dwrite-idwritefactory-createtextformat                                                                                                        |
/// | `DrawRoundedRectangle`                      | https://learn.microsoft.com/en-us/dotnet/api/system.windows.media.drawingcontext.drawroundedrectangle                                                                                                       |
/// | `ID2D1RenderTarget::DrawText`               | https://learn.microsoft.com/en-us/windows/win32/api/d2d1/nf-d2d1-id2d1rendertarget-drawtext(constwchar_uint32_idwritetextformat_constd2d1_rect_f__id2d1brush_d2d1_draw_text_options_dwrite_measuring_mode)  |
/// | `IDWriteTextFormat::SetParagraphAlignment`  | https://learn.microsoft.com/en-us/windows/win32/api/dwrite/nf-dwrite-idwritetextformat-setparagraphalignment                                                                                                |
/// | `IDWriteTextFormat::SetTextAlignment`       | https://learn.microsoft.com/en-us/windows/win32/api/dwrite/nf-dwrite-idwritetextformat-settextalignment                                                                                                     |
/// | `IDWriteTextFormat::SetWordWrapping`        | https://learn.microsoft.com/en-us/windows/win32/api/dwrite/nf-dwrite-idwritetextformat-setwordwrapping                                                                                                      |
/// | `ID2D1RenderTarget::EndDraw`                | https://learn.microsoft.com/en-us/windows/win32/api/d2d1/nf-d2d1-id2d1rendertarget-enddraw                                                                                                                  |
/// | `InvalidateRect`                            | https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-invalidaterect                                                                                                                       |
/// | `IUnknown::Release`                         | https://learn.microsoft.com/en-us/windows/win32/api/unknwn/nf-unknwn-iunknown-release                                                                                                                       |
/// | `wcslen`                                    | https://en.cppreference.com/w/c/string/wide/wcslen                                                                                                                                                          |
///
/// @file    mvcView.cpp
/// @author  Mark Nelson <marknels@hawaii.edu>
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

const int giWindowWidth  = COL0 + ( BOX_WIDTH  * 4 ) + ( GAP_WIDTH  * 3 ) + BOX_WIDTH;
const int giWindowHeight = ROW0 + ( BOX_HEIGHT * 4 ) + ( GAP_HEIGHT * 3 ) + BOX_HEIGHT + 50;  // The title is 25px and the menu is 25px

/// Holds the location and display data for each key (button) on the keypad
///
/// The order of the fields in the `struct` were suggested by clang-tidy
/// (to minimize padding)
typedef struct {
   size_t row;          ///< Row index into gDtmfTones
   size_t column;       ///< Column index into gDtmfTones
   LONG   x;            ///< The upper-left corner of the digit's box
   LONG   y;            ///< The upper-left corner of the digit's box
   WCHAR  digit[ 2 ];   ///< The digit to print
   WCHAR  letters[ 5 ]; ///< The letters above the digit
} keypad_t;


/// An array of information (location and display data) for each key (button)
/// on the keypad
static keypad_t keypad[ 16 ] = {
   {0, 4, COL0, ROW0, L"1",     L"" },
   {0, 5, COL1, ROW0, L"2",  L"ABC" },
   {0, 6, COL2, ROW0, L"3",  L"DEF" },
   {0, 7, COL3, ROW0, L"A",     L"" },
   {1, 4, COL0, ROW1, L"4",  L"GHI" },
   {1, 5, COL1, ROW1, L"5",  L"JKL" },
   {1, 6, COL2, ROW1, L"6",  L"MNO" },
   {1, 7, COL3, ROW1, L"B",     L"" },
   {2, 4, COL0, ROW2, L"7", L"PQRS" },
   {2, 5, COL1, ROW2, L"8",  L"TUV" },
   {2, 6, COL2, ROW2, L"9", L"WXYZ" },
   {2, 7, COL3, ROW2, L"C",     L"" },
   {3, 4, COL0, ROW3, L"*",     L"" },
   {3, 5, COL1, ROW3, L"0",     L"" },
   {3, 6, COL2, ROW3, L"#",     L"" },
   {3, 7, COL3, ROW3, L"D",     L"" }
};


/// Initialize all of the resources needed to draw the main window
///
/// @return `TRUE` if successful.  `FALSE` if there was a problem.
BOOL mvcViewInit() {
   HRESULT hr;  // HRESULT result
   BOOL    br;  // BOOL result

   _ASSERTE( ghMainWindow != NULL );

   /// Initialize Direct2D
   _ASSERTE( spD2DFactory == NULL );
   hr = D2D1CreateFactory( D2D1_FACTORY_TYPE_MULTI_THREADED, &spD2DFactory );
   CHECK_HR_R( IDS_VIEW_FAILED_TO_CREATE_DIRECT2D_FACTORY );  // "Failed to create Direct2D Factory"
   _ASSERTE( spD2DFactory != NULL );

   /// Initialize DirectWrite
   _ASSERTE( spDWriteFactory == NULL );
   hr = DWriteCreateFactory(
      DWRITE_FACTORY_TYPE_SHARED,                       // Is factory shared or isolated
      __uuidof( IDWriteFactory ),                       // GUID that identifies the DirectWrite factory interface
      reinterpret_cast<IUnknown**>( &spDWriteFactory )  // An address of a pointer to the newly created DirectWrite factory object
   );
   CHECK_HR_R( IDS_VIEW_FAILED_TO_CREATE_DIRECTWRITE_FACTORY );  // "Failed to create DirectWrite Factory"
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
   CHECK_HR_R( IDS_VIEW_FAILED_TO_CREATE_FONT_RESOURCE );  // "Failed to create a font resource"
   _ASSERTE( spDigitTextFormat != NULL );

   hr = spDigitTextFormat->SetWordWrapping( DWRITE_WORD_WRAPPING_WRAP );
   CHECK_HR_R( IDS_VIEW_FAILED_TO_SET_WORD_WRAP );  // "Failed to set word wrap"

   hr = spDigitTextFormat->SetTextAlignment( DWRITE_TEXT_ALIGNMENT_CENTER );  // Horizontal alignment
   CHECK_HR_R( IDS_VIEW_FAILED_TO_SET_TEXT_ALIGNMENT );  // "Failed to set text alignment"

   hr = spDigitTextFormat->SetParagraphAlignment( DWRITE_PARAGRAPH_ALIGNMENT_CENTER );  // Vertical alignment
   CHECK_HR_R( IDS_VIEW_FAILED_TO_SET_PARAGRAPH_ALIGNMENT );  // "Failed to set paragraph alignment"

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
   CHECK_HR_R( IDS_VIEW_FAILED_TO_CREATE_FONT_RESOURCE );  // "Failed to create a font resource"
   _ASSERTE( spLettersTextFormat != NULL );

   hr = spLettersTextFormat->SetWordWrapping( DWRITE_WORD_WRAPPING_WRAP );
   CHECK_HR_R( IDS_VIEW_FAILED_TO_SET_WORD_WRAP );  // "Failed to set word wrap"

   hr = spLettersTextFormat->SetTextAlignment( DWRITE_TEXT_ALIGNMENT_CENTER );  // Horizontal alignment
   CHECK_HR_R( IDS_VIEW_FAILED_TO_SET_TEXT_ALIGNMENT );  // "Failed to set text alignment"

   hr = spLettersTextFormat->SetParagraphAlignment( DWRITE_PARAGRAPH_ALIGNMENT_CENTER );  // Vertical alignment
   CHECK_HR_R( IDS_VIEW_FAILED_TO_SET_PARAGRAPH_ALIGNMENT );  // "Failed to set paragraph alignment"

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
   CHECK_HR_R( IDS_VIEW_FAILED_TO_CREATE_FONT_RESOURCE );  // "Failed to create a font resource"
   _ASSERTE( spFreqTextFormat != NULL );

   hr = spFreqTextFormat->SetWordWrapping( DWRITE_WORD_WRAPPING_WRAP );
   CHECK_HR_R( IDS_VIEW_FAILED_TO_SET_WORD_WRAP );  // "Failed to set word wrap"

   hr = spFreqTextFormat->SetTextAlignment( DWRITE_TEXT_ALIGNMENT_TRAILING );  // Horizontal alignment
   CHECK_HR_R( IDS_VIEW_FAILED_TO_SET_TEXT_ALIGNMENT );  // "Failed to set text alignment"

   hr = spFreqTextFormat->SetParagraphAlignment( DWRITE_PARAGRAPH_ALIGNMENT_CENTER );  // Vertical alignment
   CHECK_HR_R( IDS_VIEW_FAILED_TO_SET_PARAGRAPH_ALIGNMENT );  // "Failed to set paragraph alignment"

   /// Create the Direct2D render target
   RECT clientRectangle ;
   br = GetClientRect( ghMainWindow, &clientRectangle );
   CHECK_BR_R( IDS_VIEW_FAILED_TO_GET_WINDOW_SIZE );  // "Failed to get the window size"

   _ASSERTE( spRenderTarget == NULL );
   hr = spD2DFactory->CreateHwndRenderTarget(
      D2D1::RenderTargetProperties(),
      D2D1::HwndRenderTargetProperties( ghMainWindow, D2D1::SizeU( clientRectangle.right - clientRectangle.left, clientRectangle.bottom - clientRectangle.top ) ),
      &spRenderTarget
   );
   CHECK_HR_R( IDS_VIEW_FAILED_TO_CREATE_DIRECT2D_RENDER_TARGET );  // "Failed to create Direct2D Render Target"
   _ASSERTE( spRenderTarget != NULL );


   /// Create the colors (brushes) for the foreground, highlight and background
   _ASSERTE( spBrushForeground == NULL );
   hr = spRenderTarget->CreateSolidColorBrush(
      D2D1::ColorF( D2D1::ColorF( FOREGROUND_COLOR, 1.0f ) ),
      &spBrushForeground
   );
   CHECK_HR_R( IDS_VIEW_FAILED_TO_CREATE_DIRECT2D_BRUSH );  // "Failed to create Direct2D Brush"
   _ASSERTE( spBrushForeground != NULL );


   _ASSERTE( spBrushHighlight == NULL );
   hr = spRenderTarget->CreateSolidColorBrush(
      D2D1::ColorF( D2D1::ColorF( HIGHLIGHT_COLOR, 1.0f ) ),
      &spBrushHighlight
   );
   CHECK_HR_R( IDS_VIEW_FAILED_TO_CREATE_DIRECT2D_BRUSH );  // "Failed to create Direct2D Brush"
   _ASSERTE( spBrushHighlight != NULL );


   _ASSERTE( spBrushBackground == NULL );
   hr = spRenderTarget->CreateSolidColorBrush(
      D2D1::ColorF( D2D1::ColorF( BACKGROUND_COLOR, 1.0f ) ),
      &spBrushBackground
   );
   CHECK_HR_R( IDS_VIEW_FAILED_TO_CREATE_DIRECT2D_BRUSH );  // "Failed to create Direct2D Brush"
   _ASSERTE( spBrushBackground != NULL );

   return TRUE;
}


/// Cleanup the resources (in reverse order of their creation)
///
/// The IUnknown::Release() method does not have a meaningful result code
/// @see https://learn.microsoft.com/en-us/windows/win32/api/unknwn/nf-unknwn-iunknown-release
///
/// @return `TRUE` if successful.  `FALSE` if there was a problem.
BOOL mvcViewCleanup() {
   SAFE_RELEASE( spBrushForeground );  // SAFE_RELEASE returns a reference
   SAFE_RELEASE( spBrushHighlight );   // count, but not an error code
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
/// The function is inlined for performane reasons.
///
/// @param pRect_F      Pointer to the floating-point rectangle structure that
///                     will be populated **if** the rectangle needs updating.
///                     If the rectangle does not need updating, this structure
///                     contains all zeros.
/// @param pUpdateRect  The rectangle that needs to be refreshed (from WM_PAINT)
/// @param left         The rectangle that's being made
/// @param top          The rectangle that's being made
/// @param right        The rectangle that's being made
/// @param bottom       The rectangle that's being made
/// @return `TRUE` if the rectangle that's being made has any overlap with
///         pUpdateRect.  `FALSE` if the rectangle that's being made is outside
///         of pUpdateRect (which probably means it doesn't need to be redrawn).
static __forceinline BOOL makeFloatRect(
   _Out_ D2D1_RECT_F* pRect_F,
   _In_  const RECT*  pUpdateRect,
   _In_  const LONG   left,
   _In_  const LONG   top,
   _In_  const LONG   right,
   _In_  const LONG   bottom ) {

   pRect_F->left   = 0.0;  // This is inefficient, but it suppresses a compiler
   pRect_F->top    = 0.0;  // warning and is a good practice.
   pRect_F->right  = 0.0;
   pRect_F->bottom = 0.0;

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

   // LOG_TRACE_R( IDS_VIEW_DRAW_RECTANGLE, left, top, right, bottom );  // "Draw rectangle:  left=%ld  top=%ld  right=%ld  bottom=%ld"

   return TRUE;
}


/// Paint a frequency label before each row
///
/// If a DTMF tone is detected for this frequency, then paint it using the
/// highlighted font.  Otherwise, use the normal foreground font.
///
/// This function won't update content that's outside pUpdateRect.
///
/// The function is inlined for performane reasons.
///
/// @param index The row to update
/// @param pUpdateRect The rectangle to update (from WM_PAINT)
static __forceinline void paintRowFreqs( _In_ const size_t index, _In_ const RECT* pUpdateRect ) {
   _ASSERTE( spRenderTarget      != NULL );
   _ASSERTE( spBrushHighlight    != NULL );
   _ASSERTE( spBrushForeground   != NULL );
   _ASSERTE( spLettersTextFormat != NULL );
   _ASSERTE( spFreqTextFormat    != NULL );
   _ASSERTE( pUpdateRect         != NULL );

   keypad_t* pKey  = &keypad[ ( 4 * index ) + index ];  // The diaganol DTMF digits 1, 5, 9 and D
   size_t    iFreq = pKey->row;

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


/// Paint a frequency label above each column
///
/// If a DTMF tone is detected for this frequency, then paint it using the
/// highlighted font.  Otherwise, use the normal foreground font.
///
/// This function won't update content that's outside pUpdateRect.
///
/// The function is inlined for performane reasons.
///
/// @param index The column to update
/// @param pUpdateRect The rectangle to update (from WM_PAINT)
static __forceinline void paintColFreqs( _In_ const size_t index, _In_ const RECT* pUpdateRect ) {
   _ASSERTE( spRenderTarget      != NULL );
   _ASSERTE( spBrushHighlight    != NULL );
   _ASSERTE( spBrushForeground   != NULL );
   _ASSERTE( spLettersTextFormat != NULL );
   _ASSERTE( spFreqTextFormat    != NULL );
   _ASSERTE( pUpdateRect         != NULL );

   keypad_t* pKey  = &keypad[ ( 4 * index ) + index ];  // The diaganol DTMF digits 1, 5, 9 and D
   size_t    iFreq = pKey->column;

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


/// The detailed work to paint each key on the keypad... painting the rectangle,
/// digit and letters above the digit.
///
/// If a DTMF tone is detected for this combination of frequencies, then paint
/// it using the highlighted font.  Otherwise, use the normal foreground font.
///
/// This function won't update content that's outside pUpdateRect.
///
/// The function is inlined for performane reasons.
///
/// @param index The key to repaint
/// @param pUpdateRect The rectangle to update (from WM_PAINT)
static __forceinline void paintKeys( _In_ const size_t index, _In_ const RECT* pUpdateRect ) {
   _ASSERTE( spRenderTarget      != NULL );
   _ASSERTE( spBrushHighlight    != NULL );
   _ASSERTE( spBrushForeground   != NULL );
   _ASSERTE( spDigitTextFormat   != NULL );
   _ASSERTE( spLettersTextFormat != NULL );
   _ASSERTE( pUpdateRect         != NULL );

   ID2D1SolidColorBrush* pBrush;

   if ( gDtmfTones[ keypad[ index ].row ].detected == TRUE && gDtmfTones[ keypad[ index ].column ].detected == TRUE ) {
      pBrush = spBrushHighlight;
   } else {
      pBrush = spBrushForeground;
   }

   D2D1_RECT_F drawingRect;
   UINT32      cTextLength;

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


/// Paint the main window containing the DTMF keypad
///
/// @see  http://www.catch22.net/tuts/win32/flicker-free-drawing#
///
/// @param pUpdateRect The rectangle to update (from WM_PAINT)
/// @return `TRUE` if successful.  `FALSE` if there was a problem.
BOOL mvcViewPaintWindow( _In_ const RECT* pUpdateRect ) {
   HRESULT hr;  // HRESULT result

   _ASSERTE( spRenderTarget != NULL );
   _ASSERTE( pUpdateRect != NULL );

   // LOG_TRACE_R( IDS_VIEW_UPDATE_REGION, pUpdateRect->left, pUpdateRect->top, pUpdateRect->right, pUpdateRect->bottom );  // "Update region:  (%d, %d) to (%d, %d)"

   spRenderTarget->BeginDraw();  // No return value for error checking

   /// Paint the background color into the update region.
   _ASSERTE( spBrushBackground != NULL );

   D2D1_RECT_F updateRect_F = D2D1::RectF(
      static_cast<FLOAT>( pUpdateRect->left ),
      static_cast<FLOAT>( pUpdateRect->top ),
      static_cast<FLOAT>( pUpdateRect->right ),
      static_cast<FLOAT>( pUpdateRect->bottom )
   );                       // No return value for error checking

   spRenderTarget->FillRectangle(
      updateRect_F,         // The rectangle to fill
      spBrushBackground     // The background brush
   );                       // No return value to check for erors

   /// Draw the main window... starting with the column and row labels
   ///
   /// Loops are unrolled for performance.
   ///
   /// @internal For now, mostly for the sake of efficiency, we are not checking
   ///           the results of these drawing functions.  None of the methods these
   ///           functions call/use return any error messages, so (right now)
   ///           they return `void`.
   paintColFreqs( 0, pUpdateRect );
   paintColFreqs( 1, pUpdateRect );
   paintColFreqs( 2, pUpdateRect );
   paintColFreqs( 3, pUpdateRect );

   paintRowFreqs( 0, pUpdateRect );
   paintRowFreqs( 1, pUpdateRect );
   paintRowFreqs( 2, pUpdateRect );
   paintRowFreqs( 3, pUpdateRect );

   paintKeys(  0, pUpdateRect );  paintKeys(  1, pUpdateRect );
   paintKeys(  2, pUpdateRect );  paintKeys(  3, pUpdateRect );
   paintKeys(  4, pUpdateRect );  paintKeys(  5, pUpdateRect );
   paintKeys(  6, pUpdateRect );  paintKeys(  7, pUpdateRect );
   paintKeys(  8, pUpdateRect );  paintKeys(  9, pUpdateRect );
   paintKeys( 10, pUpdateRect );  paintKeys( 11, pUpdateRect );
   paintKeys( 12, pUpdateRect );  paintKeys( 13, pUpdateRect );
   paintKeys( 14, pUpdateRect );  paintKeys( 15, pUpdateRect );

   hr = spRenderTarget->EndDraw();
   CHECK_HR_C( IDS_VIEW_FAILED_TO_END_DRAW, 0 );  // "Failed to end drawing operations on render target"

   return TRUE;
}
