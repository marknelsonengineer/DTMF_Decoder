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
#include "DTMF_Decoder.h" // Resource.h


#pragma comment(lib, "d2d1")    // Link the Diect2D library (for drawing)
#pragma comment(lib, "Dwrite")  // Link the DirectWrite library (for fonts and text)

// Global Variables
ID2D1Factory* pD2DFactory = NULL;                  /// The Direct2D Factory
ID2D1HwndRenderTarget* pRenderTarget = NULL;	      /// Render target
ID2D1SolidColorBrush* gpBrushBackground = NULL;    /// A dark blue brush for the background
ID2D1SolidColorBrush* gpBrushForeground = NULL;    /// A light blue brush for the foreground
IDWriteFactory* g_pDWriteFactory = NULL;
IDWriteTextFormat* g_pTextFormat = NULL;

// Forward declarations of private functions in this file

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
      50.0f,                        // Size	
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
      D2D1::ColorF( D2D1::ColorF( BACKGROUND_COLOR, 1.0f ) ),
      &gpBrushBackground   /// @TODO Consider deleting if this goesn't get used
   );
   if ( FAILED( hr ) ) {
      OutputDebugStringA( APP_NAME ": Failed to create Direct2D Brush (Background)" );
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

   return TRUE;
}


BOOL mvcViewCleanupResources() {
   SAFE_RELEASE( gpBrushForeground );
   SAFE_RELEASE( gpBrushBackground );
   SAFE_RELEASE( pRenderTarget );
   SAFE_RELEASE( pD2DFactory );

   return TRUE;
}


BOOL mvcViewPaintWindow( HWND hWnd ) {
   pRenderTarget->BeginDraw() ;

// Clear to the background color
   pRenderTarget->Clear( D2D1::ColorF( BACKGROUND_COLOR, 1.0f ) );

   pRenderTarget->DrawRoundedRectangle(
      D2D1::RoundedRect(
      D2D1::RectF( 100.f, 100.f, 164.f, 164.f ),
      8.0f,
      8.0f ),
      gpBrushForeground,
      2.f ) ;

   /// @TODO Add error handling

   D2D1_RECT_F textLayoutRect = D2D1::RectF(
      static_cast<FLOAT>( 100.f ),
      static_cast<FLOAT>( 100.f ),
      static_cast<FLOAT>( 164.f ),
      static_cast<FLOAT>( 164.f )
   );

   const wchar_t* wszText = L"1";		// String to render
   UINT32 cTextLength = (UINT32) wcslen( wszText );	// Get text length

   pRenderTarget->DrawText(
      wszText,		// Text to render
      cTextLength,	// Text length
      g_pTextFormat,	// Text format
      textLayoutRect,	// The region of the window where the text will be rendered
      gpBrushForeground	// The brush used to draw the text
   );

   pRenderTarget->EndDraw();
   /// @TODO Look into error checking for these methods

   return TRUE;
}
