///////////////////////////////////////////////////////////////////////////////
//          University of Hawaii, College of Engineering
//          DTMF_Decoder - EE 469 - Fall 2022
//
/// A Windows Desktop C program that decodes DTMF tones
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

// Global Variables
ID2D1Factory*          pD2DFactory = NULL;         /// The Direct2D Factory
ID2D1HwndRenderTarget* pRenderTarget = NULL;	      /// Render target
ID2D1SolidColorBrush*  gpBrushForeground = NULL;   /// A light blue brush for the foreground
ID2D1SolidColorBrush*  gpBrushHighlight = NULL;    /// A lighter blue brush for the highlight
ID2D1SolidColorBrush*  gpBrushBackground = NULL;   /// A dark blue brush for the background
IDWriteFactory*        g_pDWriteFactory = NULL;    /// @TODO fixup these names and docs
IDWriteTextFormat*     g_pTextFormat = NULL;

#define BOX_WIDTH (64)
#define BOX_HEIGHT (64)

#define GAP_WIDTH (16)
#define GAP_HEIGHT (16)

#define ROW0 (64)
#define ROW1 (ROW0 + BOX_HEIGHT + GAP_HEIGHT)
#define ROW2 (ROW1 + BOX_HEIGHT + GAP_HEIGHT)
#define ROW3 (ROW2 + BOX_HEIGHT + GAP_HEIGHT)

#define COL0 (64)
#define COL1 (COL0 + BOX_WIDTH + GAP_WIDTH)
#define COL2 (COL1 + BOX_WIDTH + GAP_WIDTH)
#define COL3 (COL2 + BOX_WIDTH + GAP_WIDTH)

typedef struct {
   WCHAR  digit[2];
   size_t row;        // Row index into dtmfTones
   size_t column;     // Column index into dtmfTones
   char   letters[5];
   float  x;
   float  y;
} keypad_t;

keypad_t keypad[ 16 ] = {
   {L"1", 0, 4,     "", COL0, ROW0 },
   {L"2", 0, 5,  "ABC", COL1, ROW0 },
   {L"3", 0, 6,  "DEF", COL2, ROW0 },
   {L"4", 1, 4,  "GHI", COL0, ROW1 },
   {L"5", 1, 5,  "JKL", COL1, ROW1 },
   {L"6", 1, 6,  "MNO", COL2, ROW1 },
   {L"7", 2, 4, "PQRS", COL0, ROW2 },
   {L"8", 2, 5,  "TUV", COL1, ROW2 },
   {L"9", 2, 6, "WXYZ", COL2, ROW2 },
   {L"*", 3, 4,     "", COL0, ROW3 },
   {L"0", 3, 5,     "", COL1, ROW3 },
   {L"#", 3, 6,     "", COL2, ROW3 },
   {L"A", 0, 7,     "", COL3, ROW0 },
   {L"B", 1, 7,     "", COL3, ROW1 },
   {L"C", 2, 7,     "", COL3, ROW2 },
   {L"D", 3, 7,     "", COL3, ROW3 }
};

// Forward declarations of private functions in this file
BOOL paintDigit( HWND hWnd, size_t index );


BOOL mvcViewInitResources( HWND hWnd ) {
   /// Initialize Direct2D
   HRESULT hr = D2D1CreateFactory( D2D1_FACTORY_TYPE_MULTI_THREADED, &pD2DFactory );
   if ( FAILED( hr ) ) {
      OutputDebugStringA( APP_NAME ": Failed to create Direct2D Factory" );
      return FALSE;
   }

   hr = DWriteCreateFactory(
      DWRITE_FACTORY_TYPE_SHARED,
      __uuidof( IDWriteFactory ),
      reinterpret_cast<IUnknown**>( &g_pDWriteFactory )
   );
   if ( FAILED( hr ) ) {
      OutputDebugStringA( APP_NAME ": Failed to create DirectWrite Factory" );
      return FALSE;
   }

   /// Initialize DirectWrite
   hr = g_pDWriteFactory->CreateTextFormat(
      L"Segoe UI",                  // Font family name
      NULL,                         // Font collection(NULL sets it to the system font collection)
      DWRITE_FONT_WEIGHT_REGULAR,   // Weight
      DWRITE_FONT_STYLE_NORMAL,     // Style
      DWRITE_FONT_STRETCH_NORMAL,   // Stretch
      32.0f,                        // Size	
      L"en-us",                     // Local
      &g_pTextFormat                // Pointer to recieve the created object
   );
   if ( FAILED( hr ) ) {
      OutputDebugStringA( APP_NAME ": Failed to create a font resource" );
      return FALSE;
   }

   hr = g_pTextFormat->SetWordWrapping( DWRITE_WORD_WRAPPING_WRAP );
   if ( FAILED( hr ) ) {
      OutputDebugStringA( APP_NAME ": Failed to set word wrap mode" );
      return FALSE;
   }

   hr = g_pTextFormat->SetTextAlignment( DWRITE_TEXT_ALIGNMENT_CENTER );
   hr = g_pTextFormat->SetParagraphAlignment( DWRITE_PARAGRAPH_ALIGNMENT_CENTER );
   hr = g_pTextFormat->SetLineSpacing( DWRITE_LINE_SPACING_METHOD_DEFAULT, 50, 80 );
   /// @TODO Add error handling

   RECT rc ;
   if ( !GetClientRect( hWnd, &rc ) ) {   // Get the size of the drawing area of the window
      OutputDebugStringA( APP_NAME ": Failed to get the window size" );
      return FALSE;
   }

   hr = pD2DFactory->CreateHwndRenderTarget(
      D2D1::RenderTargetProperties(),
      D2D1::HwndRenderTargetProperties( hWnd, D2D1::SizeU( rc.right - rc.left, rc.bottom - rc.top ) ),
      &pRenderTarget
   );
   if ( FAILED( hr ) ) {
      OutputDebugStringA( APP_NAME ": Failed to create Direct2D Render Target" );
      return FALSE;
   }

   hr = pRenderTarget->CreateSolidColorBrush(
      D2D1::ColorF( D2D1::ColorF( FOREGROUND_COLOR, 1.0f ) ),
      &gpBrushForeground
   );
   if ( FAILED( hr ) ) {
      OutputDebugStringA( APP_NAME ": Failed to create Direct2D Brush (Foreground)" );
      return FALSE;
   }

   hr = pRenderTarget->CreateSolidColorBrush(
      D2D1::ColorF( D2D1::ColorF( HIGHLIGHT_COLOR, 1.0f ) ),
      &gpBrushHighlight
   );
   if ( FAILED( hr ) ) {
      OutputDebugStringA( APP_NAME ": Failed to create Direct2D Brush (Highlight)" );
      return FALSE;
   }

   hr = pRenderTarget->CreateSolidColorBrush(
      D2D1::ColorF( D2D1::ColorF( BACKGROUND_COLOR, 1.0f ) ),
      &gpBrushBackground   /// @TODO Consider deleting if this goesn't get used
   );
   if ( FAILED( hr ) ) {
      OutputDebugStringA( APP_NAME ": Failed to create Direct2D Brush (Background)" );
      return FALSE;
   }

   return TRUE;
}


BOOL mvcViewCleanupResources() {
   SAFE_RELEASE( gpBrushForeground );
   SAFE_RELEASE( gpBrushHighlight );
   SAFE_RELEASE( gpBrushBackground );
   SAFE_RELEASE( pRenderTarget );
   SAFE_RELEASE( pD2DFactory );

   return TRUE;
}


BOOL mvcViewPaintWindow( HWND hWnd ) {
   pRenderTarget->BeginDraw() ;

// Clear to the background color
   pRenderTarget->Clear( D2D1::ColorF( BACKGROUND_COLOR, 1.0f ) );

   for ( int i = 0 ; i < 16 ; i++ ) {
      paintDigit( hWnd, i );
   }

   pRenderTarget->EndDraw();
   /// @TODO Look into error checking for these methods

   return TRUE;
}


BOOL paintDigit( HWND hWnd, size_t index ) {

   ID2D1SolidColorBrush* pBrush = gpBrushForeground;

   if ( dtmfTones[ keypad[ index ].row ].detected == TRUE && dtmfTones[ keypad[ index ].column ].detected == TRUE ) {
      pBrush = gpBrushHighlight;
   }

   pRenderTarget->DrawRoundedRectangle(
      D2D1::RoundedRect(
      D2D1::RectF( keypad[index].x, keypad[index].y, keypad[index].x + BOX_WIDTH, keypad[index].y + BOX_HEIGHT ),
      8.0f,
      8.0f ),
      pBrush,
      2.f ) ;

   /// @TODO Add error handling

   D2D1_RECT_F textLayoutRect = D2D1::RectF(
      static_cast<FLOAT>( keypad[index].x ),
      static_cast<FLOAT>( keypad[index].y+12 ),
      static_cast<FLOAT>( keypad[index].x + BOX_WIDTH ),
      static_cast<FLOAT>( keypad[index].y+12 + BOX_HEIGHT )
   );

   //const wchar_t* wszText = L"1";		// String to render
   UINT32 cTextLength = (UINT32) wcslen( keypad[index].digit);	  // Get text length

   pRenderTarget->DrawText(
      keypad[index].digit,		// Text to render
      cTextLength,	// Text length
      g_pTextFormat,	// Text format
      textLayoutRect,	// The region of the window where the text will be rendered
      pBrush	// The brush used to draw the text
   );

   return TRUE;
}
