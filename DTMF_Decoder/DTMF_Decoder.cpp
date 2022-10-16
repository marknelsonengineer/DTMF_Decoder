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
#include "d2d1.h"         // For Direct2D
#include "DTMF_Decoder.h" // Resource.h

#pragma comment(lib, "d2d1")


#define MAX_LOADSTRING 100

// Global Variables
HINSTANCE     ghInst;                              /// Current instance
WCHAR         gszTitle[ MAX_LOADSTRING ];          /// The title bar text
WCHAR         gszWindowClass[ MAX_LOADSTRING ];    /// The main window class name
ID2D1Factory* pD2DFactory = NULL;                  /// The Direct2D Factory
HBRUSH        ghBrushBlueBackground;               /// This is the background blue, so it's always in use


// Forward declarations of functions included in this code module
LRESULT CALLBACK WndProc( HWND, UINT, WPARAM, LPARAM );
INT_PTR CALLBACK About( HWND, UINT, WPARAM, LPARAM );
VOID             Cleanup();


/// Program entrypoint
int APIENTRY wWinMain( _In_     HINSTANCE hInstance,
   _In_opt_ HINSTANCE hPrevInstance,
   _In_     LPWSTR    lpCmdLine,
   _In_     int       nCmdShow ) {
   UNREFERENCED_PARAMETER( hPrevInstance );
   UNREFERENCED_PARAMETER( lpCmdLine );

   /// Initialize global strings
   LoadStringW( hInstance, IDS_APP_TITLE, gszTitle, MAX_LOADSTRING );
   LoadStringW( hInstance, IDC_DTMFDECODER, gszWindowClass, MAX_LOADSTRING );

   // Register the Windows Class
   WNDCLASSEXW wcex;

   ghBrushBlueBackground = CreateSolidBrush( RGB( 25, 23, 55 ) );  // The background shade of blue I'm after

   wcex.cbSize = sizeof( WNDCLASSEX );
   wcex.style = CS_HREDRAW | CS_VREDRAW;
   wcex.lpfnWndProc = WndProc;
   wcex.cbClsExtra = 0;
   wcex.cbWndExtra = 0;
   wcex.hInstance = hInstance;
   wcex.hIcon = LoadIcon( hInstance, MAKEINTRESOURCE( IDI_DTMFDECODER ) );
   wcex.hCursor = LoadCursor( nullptr, IDC_ARROW );
   wcex.hbrBackground = ghBrushBlueBackground;
   wcex.lpszMenuName = MAKEINTRESOURCEW( IDC_DTMFDECODER );
   wcex.lpszClassName = gszWindowClass;
   wcex.hIconSm = LoadIcon( wcex.hInstance, MAKEINTRESOURCE( IDI_SMALL ) );

   if ( !RegisterClassExW( &wcex ) ) {
      MessageBox( NULL, TEXT( "Failed to register window class!" ), L"error", MB_ICONERROR ) ;
      return FALSE;
   }

   ghInst = hInstance; // Store the instance handle in a global variable

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
      return FALSE;
   }

   ShowWindow( hWnd, nCmdShow );
   UpdateWindow( hWnd );

   HACCEL hAccelTable = LoadAccelerators( hInstance, MAKEINTRESOURCE( IDC_DTMFDECODER ) );

   /// Create a Direct2D Factory
   HRESULT hr = D2D1CreateFactory( D2D1_FACTORY_TYPE_MULTI_THREADED, &pD2DFactory );
   if ( hr != S_OK ) {
      return FALSE;
   }

   MSG msg;

   /// Main message loop
   while ( GetMessage( &msg, nullptr, 0, 0 ) ) {
      if ( !TranslateAccelerator( msg.hwnd, hAccelTable, &msg ) ) {
         TranslateMessage( &msg );
         DispatchMessage( &msg );
      }
   }

   return (int) msg.wParam;
}


VOID Cleanup() {
//   SAFE_RELEASE( pRenderTarget ) ;
//   SAFE_RELEASE( pBlackBrush ) ;
   SAFE_RELEASE( pD2DFactory ) ;

   if ( ghBrushBlueBackground != NULL ) {
      DeleteObject( ghBrushBlueBackground );
      ghBrushBlueBackground = NULL;
   }
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
            EndPaint( hWnd, &ps );
         }
         break;
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
