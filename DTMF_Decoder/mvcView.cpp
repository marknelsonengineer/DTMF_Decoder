///////////////////////////////////////////////////////////////////////////////
//          University of Hawaii, College of Engineering
//          DTMF_Decoder - EE 469 - Fall 2022
//
/// A Windows Desktop C program that decodes DTMF tones
/// 
/// This is the implementation of the Direct2D paint commands as the view 
/// component of a model-view-controller architecture.
///
/// @file mvcView.cpp
/// @version 1.0
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

#pragma comment(lib, "d2d1")    // Link the Diect2D library (for drawing)
#pragma comment(lib, "Dwrite")  // Link the DirectWrite library (for fonts and text)

// Global Variables (private to this source file)
ID2D1Factory*          gpD2DFactory = NULL;        /// The Direct2D Factory
ID2D1HwndRenderTarget* gpRenderTarget = NULL;	   /// Render target
ID2D1SolidColorBrush*  gpBrushForeground = NULL;   /// A light blue brush for the foreground
ID2D1SolidColorBrush*  gpBrushHighlight = NULL;    /// A lighter blue brush for the highlight
ID2D1SolidColorBrush*  gpBrushBackground = NULL;   /// A dark blue brush for the background
IDWriteFactory*        gpDWriteFactory = NULL;     /// A DirectWrite factory object
IDWriteTextFormat*     gpDigitTextFormat = NULL;   /// The font for the digits
IDWriteTextFormat*     gpLettersTextFormat = NULL; /// The font for the letters above the digits (and the `Hz` units)
IDWriteTextFormat*     gpFreqTextFormat = NULL;    /// The font for the frequency

#define BOX_WIDTH (64)
#define BOX_HEIGHT (64)

#define GAP_WIDTH (16)
#define GAP_HEIGHT (16)

#define ROW0 (64)
#define ROW1 (ROW0 + BOX_HEIGHT + GAP_HEIGHT)
#define ROW2 (ROW1 + BOX_HEIGHT + GAP_HEIGHT)
#define ROW3 (ROW2 + BOX_HEIGHT + GAP_HEIGHT)

#define COL0 (96)
#define COL1 (COL0 + BOX_WIDTH + GAP_WIDTH)
#define COL2 (COL1 + BOX_WIDTH + GAP_WIDTH)
#define COL3 (COL2 + BOX_WIDTH + GAP_WIDTH)

typedef struct {
   WCHAR  digit[2];   // The digit to print
   size_t row;        // Row index into dtmfTones
   size_t column;     // Column index into dtmfTones
   WCHAR  letters[5]; // The letters above the digit
   float  x;          // The upper-left corner of the digit's box
   float  y;          // The upper-left corner of the digit's box
} keypad_t;

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
BOOL paintDigit( HWND hWnd, size_t index );
BOOL paintRowFreq( HWND hWnd, size_t index );
BOOL paintColFreq( HWND hWnd, size_t index );


BOOL mvcViewInitResources( HWND hWnd ) {
   /// Initialize Direct2D
   HRESULT hr = D2D1CreateFactory( D2D1_FACTORY_TYPE_MULTI_THREADED, &gpD2DFactory );
   if ( FAILED( hr ) ) {
      OutputDebugStringA( APP_NAME ": Failed to create Direct2D Factory" );
      return FALSE;
   }

   /// Initialize DirectWrite
   hr = DWriteCreateFactory(
      DWRITE_FACTORY_TYPE_SHARED,
      __uuidof( IDWriteFactory ),
      reinterpret_cast<IUnknown**>( &gpDWriteFactory )
   );
   if ( FAILED( hr ) ) {
      OutputDebugStringA( APP_NAME ": Failed to create DirectWrite Factory" );
      return FALSE;
   }

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
   if ( FAILED( hr ) ) {
      OutputDebugStringA( APP_NAME ": Failed to create a font resource (digit text format)" );
      return FALSE;
   }

   hr = gpDigitTextFormat->SetWordWrapping( DWRITE_WORD_WRAPPING_WRAP );
   if ( FAILED( hr ) ) {
      OutputDebugStringA( APP_NAME ": Failed to set word wrap mode (digit text format)" );
      return FALSE;
   }

   hr = gpDigitTextFormat->SetTextAlignment( DWRITE_TEXT_ALIGNMENT_CENTER );  // Horizontal alignment
   hr = gpDigitTextFormat->SetParagraphAlignment( DWRITE_PARAGRAPH_ALIGNMENT_CENTER );  // Vertical alignment
   /// @TODO Add error handling


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
   if ( FAILED( hr ) ) {
      OutputDebugStringA( APP_NAME ": Failed to create a font resource (letters text format)" );
      return FALSE;
   }

   hr = gpLettersTextFormat->SetWordWrapping( DWRITE_WORD_WRAPPING_WRAP );
   if ( FAILED( hr ) ) {
      OutputDebugStringA( APP_NAME ": Failed to set word wrap mode (letters text format)" );
      return FALSE;
   }

   hr = gpLettersTextFormat->SetTextAlignment( DWRITE_TEXT_ALIGNMENT_CENTER );  // Horizontal alignment
   hr = gpLettersTextFormat->SetParagraphAlignment( DWRITE_PARAGRAPH_ALIGNMENT_CENTER );  // Vertical alignment
   /// @TODO Add error handling

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
   if ( FAILED( hr ) ) {
      OutputDebugStringA( APP_NAME ": Failed to create a font resource (frequency text format)" );
      return FALSE;
   }

   hr = gpFreqTextFormat->SetWordWrapping( DWRITE_WORD_WRAPPING_WRAP );
   if ( FAILED( hr ) ) {
      OutputDebugStringA( APP_NAME ": Failed to set word wrap mode (frequency text format)" );
      return FALSE;
   }

   hr = gpFreqTextFormat->SetTextAlignment( DWRITE_TEXT_ALIGNMENT_TRAILING );  // Horizontal alignment
   hr = gpFreqTextFormat->SetParagraphAlignment( DWRITE_PARAGRAPH_ALIGNMENT_CENTER );  // Vertical alignment
   /// @TODO Add error handling

   /// Create the render target
   RECT rc ;
   if ( !GetClientRect( hWnd, &rc ) ) {   // Get the size of the drawing area of the window
      OutputDebugStringA( APP_NAME ": Failed to get the window size" );
      return FALSE;
   }

   hr = gpD2DFactory->CreateHwndRenderTarget(
      D2D1::RenderTargetProperties(),
      D2D1::HwndRenderTargetProperties( hWnd, D2D1::SizeU( rc.right - rc.left, rc.bottom - rc.top ) ),
      &gpRenderTarget
   );
   if ( FAILED( hr ) ) {
      OutputDebugStringA( APP_NAME ": Failed to create Direct2D Render Target" );
      return FALSE;
   }

   /// Create the colors (brushes) for the foreground, highlight and background
   hr = gpRenderTarget->CreateSolidColorBrush(
      D2D1::ColorF( D2D1::ColorF( FOREGROUND_COLOR, 1.0f ) ),
      &gpBrushForeground
   );
   if ( FAILED( hr ) ) {
      OutputDebugStringA( APP_NAME ": Failed to create Direct2D Brush (Foreground)" );
      return FALSE;
   }

   hr = gpRenderTarget->CreateSolidColorBrush(
      D2D1::ColorF( D2D1::ColorF( HIGHLIGHT_COLOR, 1.0f ) ),
      &gpBrushHighlight
   );
   if ( FAILED( hr ) ) {
      OutputDebugStringA( APP_NAME ": Failed to create Direct2D Brush (Highlight)" );
      return FALSE;
   }

   hr = gpRenderTarget->CreateSolidColorBrush(
      D2D1::ColorF( D2D1::ColorF( BACKGROUND_COLOR, 1.0f ) ),
      &gpBrushBackground   /// @TODO Consider deleting if this goesn't get used
   );
   if ( FAILED( hr ) ) {
      OutputDebugStringA( APP_NAME ": Failed to create Direct2D Brush (Background)" );
      return FALSE;
   }

   return TRUE;
}


/// Cleanup the resources (in reverse order of their creation)
BOOL mvcViewCleanupResources() {
   SAFE_RELEASE( gpBrushForeground );
   SAFE_RELEASE( gpBrushHighlight );
   SAFE_RELEASE( gpBrushBackground );
   SAFE_RELEASE( gpRenderTarget );
   SAFE_RELEASE( gpD2DFactory );

   return TRUE;
}


BOOL mvcViewPaintWindow( HWND hWnd ) {
   gpRenderTarget->BeginDraw() ;

   // Clear to the background color
   gpRenderTarget->Clear( D2D1::ColorF( BACKGROUND_COLOR, 1.0f ) );

   // Paint each of the digits
   for ( int i = 0 ; i < 16 ; i++ ) {
      paintDigit( hWnd, i );
   }

   for ( int i = 0 ; i < 4 ; i++ ) {
      paintRowFreq( hWnd, i );
      paintColFreq( hWnd, i );
   }

   gpRenderTarget->EndDraw();
   /// @TODO Look into error checking for these methods

   return TRUE;
}

BOOL paintRowFreq( HWND hWnd, size_t index ) {
   keypad_t* pKey = &keypad[ (4 * index) + index ];  // The diaganol DTMF digits 1, 5, 9 and D
   size_t iFreq = pKey->row;

   ID2D1SolidColorBrush* pBrush = gpBrushForeground;

   if ( dtmfTones[ iFreq ].detected == TRUE ) {
      pBrush = gpBrushHighlight;
   }

   D2D1_RECT_F freqTextRect = D2D1::RectF(
      static_cast<FLOAT>( COL0 - 78 ),
      static_cast<FLOAT>( pKey->y ),
      static_cast<FLOAT>( COL0 - 32 ),
      static_cast<FLOAT>( pKey->y + BOX_HEIGHT )
   );

   //const wchar_t* wszText = L"1";		// String to render
   UINT32 cTextLength = (UINT32) wcslen( dtmfTones[ iFreq ].label );	  // Get text length

   gpRenderTarget->DrawText(
      dtmfTones[ iFreq ].label, // Text to render
      cTextLength,              // Text length
      gpFreqTextFormat,         // Text format
      freqTextRect,	           // The region of the window where the text will be rendered
      pBrush                    // The brush used to draw the text
   );

   freqTextRect = D2D1::RectF(
      static_cast<FLOAT>( COL0 - 32 ),
      static_cast<FLOAT>( pKey->y - 4 ),
      static_cast<FLOAT>( COL0 - 12 ),
      static_cast<FLOAT>( pKey->y + BOX_HEIGHT - 4)
   );

   const wchar_t* wszText = L"Hz";		        // String to render
   cTextLength = (UINT32) wcslen( wszText );	  // Get text length

   gpRenderTarget->DrawText(
      wszText,                  // Text to render
      cTextLength,              // Text length
      gpLettersTextFormat,         // Text format
      freqTextRect,	           // The region of the window where the text will be rendered
      pBrush                    // The brush used to draw the text
   );

   return TRUE;
}


BOOL paintColFreq( HWND hWnd, size_t index ) {
   keypad_t* pKey = &keypad[ ( 4 * index ) + index ];  // The diaganol DTMF digits 1, 5, 9 and D
   size_t iFreq = pKey->column;

   ID2D1SolidColorBrush* pBrush = gpBrushForeground;

   if ( dtmfTones[ iFreq ].detected == TRUE ) {
      pBrush = gpBrushHighlight;
   }

   D2D1_RECT_F freqTextRect = D2D1::RectF(
      static_cast<FLOAT>( pKey->x - 16 ),
      static_cast<FLOAT>( ROW0 - 48 ),
      static_cast<FLOAT>( pKey->x + BOX_WIDTH - 16 ),
      static_cast<FLOAT>( ROW0 )
   );

   UINT32 cTextLength = (UINT32) wcslen( dtmfTones[ iFreq ].label );	  // Get text length

   gpRenderTarget->DrawText(
      dtmfTones[ iFreq ].label, // Text to render
      cTextLength,              // Text length
      gpFreqTextFormat,         // Text format
      freqTextRect,	           // The region of the window where the text will be rendered
      pBrush                    // The brush used to draw the text
   );

   freqTextRect = D2D1::RectF(
      static_cast<FLOAT>( pKey->x + 47 ),
      static_cast<FLOAT>( ROW0 - 36 ),
      static_cast<FLOAT>( pKey->x + 71 ),
      static_cast<FLOAT>( ROW0 - 20 )
   );

   const wchar_t* wszText = L"Hz";		        // String to render
   cTextLength = (UINT32) wcslen( wszText );	  // Get text length

   gpRenderTarget->DrawText(
      wszText,                  // Text to render
      cTextLength,              // Text length
      gpLettersTextFormat,         // Text format
      freqTextRect,	           // The region of the window where the text will be rendered
      pBrush                    // The brush used to draw the text
   );

   return TRUE;
}


// The detailed work to paint a digit
BOOL paintDigit( HWND hWnd, size_t index ) {
   ID2D1SolidColorBrush* pBrush = gpBrushForeground;

   if ( dtmfTones[ keypad[ index ].row ].detected == TRUE && dtmfTones[ keypad[ index ].column ].detected == TRUE ) {
      pBrush = gpBrushHighlight;
   }

   gpRenderTarget->DrawRoundedRectangle(
      D2D1::RoundedRect(
      D2D1::RectF( keypad[index].x, keypad[index].y, keypad[index].x + BOX_WIDTH, keypad[index].y + BOX_HEIGHT ),
      8.0f,
      8.0f ),
      pBrush,
      2.f ) ;

   /// @TODO Add error handling

   D2D1_RECT_F digitTextRect = D2D1::RectF(
      static_cast<FLOAT>( keypad[index].x ),
      static_cast<FLOAT>( keypad[index].y + 24 ),
      static_cast<FLOAT>( keypad[index].x + BOX_WIDTH ),
      static_cast<FLOAT>( keypad[index].y + BOX_HEIGHT - 6 )
   );

   //const wchar_t* wszText = L"1";		// String to render
   UINT32 cTextLength = (UINT32) wcslen( keypad[index].digit);	  // Get text length

   gpRenderTarget->DrawText(
      keypad[index].digit, // Text to render
      cTextLength,         // Text length
      gpDigitTextFormat,   // Text format
      digitTextRect,	      // The region of the window where the text will be rendered
      pBrush               // The brush used to draw the text
   );


   // Draw the letters above the digits
   cTextLength = (UINT32) wcslen( keypad[ index ].letters );	  // Get text length

   if ( cTextLength > 0 ) {
      D2D1_RECT_F lettersTextRect = D2D1::RectF(
         static_cast<FLOAT>( keypad[ index ].x ),
         static_cast<FLOAT>( keypad[ index ].y + 6 ),
         static_cast<FLOAT>( keypad[ index ].x + BOX_WIDTH ),
         static_cast<FLOAT>( keypad[ index ].y + 6 + 16 )
      );

      gpRenderTarget->DrawText(
         keypad[ index ].letters,  // Text to render
         cTextLength,              // Text length
         gpLettersTextFormat,      // Text format
         lettersTextRect,          // The region of the window where the text will be rendered
         pBrush                    // The brush used to draw the text
      );
   }

   return TRUE;
}
