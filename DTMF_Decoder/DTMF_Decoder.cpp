///////////////////////////////////////////////////////////////////////////////
//          University of Hawaii, College of Engineering
//          DTMF_Decoder - EE 469 - Fall 2022
//
/// A Windows Desktop C program that decodes DTMF tones
///
/// @file DTMF_Decoder.cpp
/// @version 1.0
///
/// @author Mark Nelson <marknels@hawaii.edu>
/// @date   10_Oct_2022
///////////////////////////////////////////////////////////////////////////////

#include "framework.h"    // Standard system include files
#include <d2d1.h>         // For Direct2D (drawing)
#include <dwrite.h>       // For DirectWrite (fonts and text)
#include "DTMF_Decoder.h" // Resource.h

#pragma comment(lib, "d2d1")    // Link the Diect2D library (for drawing)
#pragma comment(lib, "Dwrite")  // Link the DirectWrite library (for fonts and text)


#define MAX_LOADSTRING    (100)

#define FOREGROUND_COLOR  (0x63B5FE)
#define BACKGROUND_COLOR  (0x181737)

// Global Variables
HINSTANCE     ghInst = NULL;                       /// Current instance
WCHAR         gszTitle[ MAX_LOADSTRING ];          /// The title bar text
WCHAR         gszWindowClass[ MAX_LOADSTRING ];    /// The main window class name
ID2D1Factory* pD2DFactory = NULL;                  /// The Direct2D Factory
ID2D1HwndRenderTarget* pRenderTarget = NULL;	      /// Render target
ID2D1SolidColorBrush* gpBrushBackground = NULL;    /// A dark blue brush for the background
ID2D1SolidColorBrush* gpBrushForeground = NULL;    /// A light blue brush for the foreground
IDWriteFactory*       g_pDWriteFactory = NULL;
IDWriteTextFormat*    g_pTextFormat = NULL;


// Forward declarations of functions included in this code module
LRESULT CALLBACK WndProc( HWND, UINT, WPARAM, LPARAM );
INT_PTR CALLBACK About( HWND, UINT, WPARAM, LPARAM );
VOID             Cleanup();
VOID             DrawRectangle( HWND );


/// Program entrypoint
int APIENTRY wWinMain( 
   _In_     HINSTANCE hInstance,
   _In_opt_ HINSTANCE hPrevInstance,
   _In_     LPWSTR    lpCmdLine,
   _In_     int       nCmdShow ) {

   OutputDebugStringA( APP_NAME ": Starting" );

   ghInst = hInstance; // Store the instance handle in a global variable


   /// Initialize global strings
   LoadStringW( hInstance, IDS_APP_TITLE, gszTitle, MAX_LOADSTRING );
   LoadStringW( hInstance, IDC_DTMFDECODER, gszWindowClass, MAX_LOADSTRING );


   // Register the Windows Class
   WNDCLASSEXW wcex;
   ZeroMemory( &wcex, sizeof( wcex ) );

   wcex.cbSize = sizeof( WNDCLASSEX );
   wcex.style = CS_HREDRAW | CS_VREDRAW;
   wcex.lpfnWndProc = WndProc;
   wcex.cbClsExtra = 0;
   wcex.cbWndExtra = 0;
   wcex.hInstance = hInstance;
   wcex.hIcon = LoadIcon( hInstance, MAKEINTRESOURCE( IDI_DTMFDECODER ) );
   wcex.hCursor = LoadCursor( nullptr, IDC_ARROW );
   wcex.hbrBackground = (HBRUSH) ( COLOR_WINDOW + 1 );
   wcex.lpszMenuName = MAKEINTRESOURCEW( IDC_DTMFDECODER );
   wcex.lpszClassName = gszWindowClass;
   wcex.hIconSm = LoadIcon( wcex.hInstance, MAKEINTRESOURCE( IDI_SMALL ) );

   if ( !RegisterClassExW( &wcex ) ) {
      OutputDebugStringA( APP_NAME ": Failed to register window class");
      return FALSE;
   }

   HWND hWnd = CreateWindowW(
      gszWindowClass,
      gszTitle,
      WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
      CW_USEDEFAULT,          // X position of window
      0,                      // Y position of window
      640,                    // Width of window
      640,                    // Height of window
      nullptr,                // hWndParent
      nullptr,                // hMenu
      hInstance,              // hInstance
      nullptr );              // lpParam

   if ( !hWnd ) {
      OutputDebugStringA( APP_NAME ": Failed to create main window" );
      return FALSE;
   }

   /// Initialize Direct2D
   HRESULT hr = D2D1CreateFactory( D2D1_FACTORY_TYPE_MULTI_THREADED, &pD2DFactory );
   if ( FAILED(hr) ) {
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


   /// Initialize Win32-isms
   ShowWindow( hWnd, nCmdShow );   // It's OK to ignore the result of this
   if ( !UpdateWindow( hWnd ) ) {
      OutputDebugStringA( APP_NAME ": Failed to do the initial window update" );
      return FALSE;
   }

   HACCEL hAccelTable = LoadAccelerators( hInstance, MAKEINTRESOURCE( IDC_DTMFDECODER ) );
   if ( hAccelTable == NULL ) {
      OutputDebugStringA( APP_NAME ": Failed to load manu accelerator" );
      return FALSE;
   }

   OutputDebugStringA( APP_NAME ": All global resources were successfully initialized" );


   /// Main message loop
   MSG msg;
   ZeroMemory( &msg, sizeof( msg ) );


   while ( GetMessage( &msg, nullptr, 0, 0 ) ) {
      if ( !TranslateAccelerator( msg.hwnd, hAccelTable, &msg ) ) {
         TranslateMessage( &msg );
         DispatchMessage( &msg );
      }
   }

   return (int) msg.wParam;
}


VOID Cleanup() {
   SAFE_RELEASE( gpBrushForeground );
   SAFE_RELEASE( gpBrushBackground );
   SAFE_RELEASE( pRenderTarget );
   SAFE_RELEASE( pD2DFactory );

   OutputDebugStringA( APP_NAME ": Ending" );

}


/// Message handler for the main window
LRESULT CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam ) {
   switch ( message ) {
      case WM_COMMAND:  /// WM_COMMAND - Process the application menu
         {
            int wmId = LOWORD( wParam );
            // Parse the menu selections
            switch ( wmId ) {
               case IDM_ABOUT:
                  DialogBox( ghInst, MAKEINTRESOURCE( IDD_ABOUTBOX ), hWnd, About );
                  break;
               case IDM_EXIT:
                  DestroyWindow( hWnd );

                  break;
               default:
                  return DefWindowProc( hWnd, message, wParam, lParam );
            }
         }
         break;
      case WM_PAINT:  /// WM_PAINT - Paint the main window
         {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint( hWnd, &ps );

            // TODO: Add any drawing code that uses hdc here...
            DrawRectangle( hWnd ) ;

            EndPaint( hWnd, &ps );
         }
         break;
      case WM_KEYDOWN:  /// WM_KEYDOWN - Exit if <ESC> is pressed
         {
            switch ( wParam ) {
               case VK_ESCAPE:
                  SendMessage( hWnd, WM_CLOSE, 0, 0 );
                  break ;
               default:
                  break ;
            }
         }
         break ;
      case WM_DESTROY:  /// WM_DESTROY - Post a quit message and return
         Cleanup();
         PostQuitMessage( 0 );
         break;
      default:
         return DefWindowProc( hWnd, message, wParam, lParam );
   }
   return 0;
}


// Message handler for the About box
INT_PTR CALLBACK About( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam ) {
   UNREFERENCED_PARAMETER( lParam );
   switch ( message ) {
      case WM_INITDIALOG:
         return (INT_PTR) TRUE;

      case WM_COMMAND:
         if ( LOWORD( wParam ) == IDOK || LOWORD( wParam ) == IDCANCEL ) {
            EndDialog( hDlg, LOWORD( wParam ) );
            return (INT_PTR) TRUE;
         }
         break;
   }
   return (INT_PTR) FALSE;
}


VOID DrawRectangle( HWND hWnd ) {
   pRenderTarget->BeginDraw() ;

   // Clear to the background color
   pRenderTarget->Clear( D2D1::ColorF( BACKGROUND_COLOR, 1.0f ) );
   
   pRenderTarget->DrawRoundedRectangle(
      D2D1::RoundedRect(
         D2D1::RectF( 100.f, 100.f, 164.f, 164.f ),
         8.0f,
         8.0f ),
      gpBrushForeground, 
      2.f) ;
   
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
}
