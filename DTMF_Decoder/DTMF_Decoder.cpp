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
#include "mvcView.h"      // For drawing the window
#include "mvcModel.h"     // Holds the persistent model for the application
#include "audio.h"        // For capturing audio
#include <combaseapi.h>   // For initializing COM
#include "DTMF_Decoder.h" // Resource.h


#define MAX_LOADSTRING    (100)


// Global Variables
HINSTANCE ghInst = NULL;                       /// Current instance
WCHAR     gszTitle[ MAX_LOADSTRING ];          /// The title bar text
WCHAR     gszWindowClass[ MAX_LOADSTRING ];    /// The main window class name


// Forward declarations of private functions in this file
LRESULT CALLBACK WndProc( HWND, UINT, WPARAM, LPARAM );
INT_PTR CALLBACK About( HWND, UINT, WPARAM, LPARAM );
VOID             DrawRectangle( HWND );


/// Program entrypoint
int APIENTRY wWinMain( 
   _In_     HINSTANCE hInstance,
   _In_opt_ HINSTANCE hPrevInstance,
   _In_     LPWSTR    lpCmdLine,
   _In_     int       nCmdShow ) {

   OutputDebugStringA( APP_NAME ": Starting" );

   ghInst = hInstance; /// Store the instance handle in a global variable

   /// Initialize COM (needs to be called once per each thread)
   HRESULT hr = CoInitializeEx( NULL, COINIT_APARTMENTTHREADED );
   if ( hr != S_OK ) {
      OutputDebugStringA( __FUNCTION__ ":  Failed to initialize COM" );
      return FALSE;
   }

   /// Initialize global strings
   LoadStringW( hInstance, IDS_APP_TITLE, gszTitle, MAX_LOADSTRING );
   LoadStringW( hInstance, IDC_DTMFDECODER, gszWindowClass, MAX_LOADSTRING );


   /// Register the Windows Class
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
      OutputDebugStringA( APP_NAME ": Failed to register window class.  Exiting.");
      return FALSE;
   }

   /// Create the window
   HWND hWnd = CreateWindowW(
      gszWindowClass,
      gszTitle,
      WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
      CW_USEDEFAULT,          // X position of window
      0,                      // Y position of window
      windowWidth,            // Width of window
      windowHeight,           // Height of window
      nullptr,                // hWndParent
      nullptr,                // hMenu
      hInstance,              // hInstance
      nullptr );              // lpParam

   if ( !hWnd ) {
      OutputDebugStringA( APP_NAME ": Failed to create main window.  Exiting." );
      return FALSE;
   }

   /// Initialize the view
   if( !mvcViewInitResources( hWnd ) ) {
      OutputDebugStringA( APP_NAME ": Failed to do initialize the view.  Exiting." );
      return FALSE;
   }

   if ( !initAudioDevice( hWnd ) ) {
      OutputDebugStringA( APP_NAME ": Failed to do initialize the audio system.  Exiting." );
      return FALSE;
   }

   /// Initialize Win32-isms
   ShowWindow( hWnd, nCmdShow );   // It's OK to ignore the result of this
   if ( !UpdateWindow( hWnd ) ) {
      OutputDebugStringA( APP_NAME ": Failed to do the initial window update.  Exiting." );
      return FALSE;
   }

   HACCEL hAccelTable = LoadAccelerators( hInstance, MAKEINTRESOURCE( IDC_DTMFDECODER ) );
   if ( hAccelTable == NULL ) {
      OutputDebugStringA( APP_NAME ": Failed to load manu accelerator.  Exiting." );
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

   /// Cleanup all resources
   cleanupAudioDevice();

   CoUninitialize();  /// Unwind COM

   OutputDebugStringA( APP_NAME ": All global resources were cleaned up.  Ending program." );

   return (int) msg.wParam;
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
            mvcViewPaintWindow( hWnd ) ;

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
      case WM_CLOSE:    /// WM_CLOSE - Start the process of closing the application
         {
            isRunning = false;
            if ( gAudioSamplesReadyEvent != NULL ) {
               SetEvent( gAudioSamplesReadyEvent );
            }

            stopAudioDevice( hWnd );

            DestroyWindow( hWnd );

            break;
         }
      case WM_DESTROY:  /// WM_DESTROY - Post a quit message and return
         mvcViewCleanupResources();

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
