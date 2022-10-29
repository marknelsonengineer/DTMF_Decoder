///////////////////////////////////////////////////////////////////////////////
//          University of Hawaii, College of Engineering
//          DTMF_Decoder - EE 469 - Fall 2022
//
/// A Windows Desktop C program that decodes DTMF tones
///
/// The main loop (with wWinMain and WndProc)
///
/// @file    DTMF_Decoder.cpp
/// @version 1.0
///
/// @author Mark Nelson <marknels@hawaii.edu>
/// @date   10_Oct_2022
///////////////////////////////////////////////////////////////////////////////

#include "framework.h"    // Standard system include files
#include <combaseapi.h>   // For initializing COM

#include "DTMF_Decoder.h" // For APP_NAME
#include "mvcModel.h"     // Holds the persistent model for the application
#include "mvcView.h"      // For drawing the window
#include "audio.h"        // For capturing audio
#include "goertzel.h"     // For goertzel_end()
#include "resource.h"     // For the resource definitions


/// Defines the size of the wide-string buffer used to get strings from the
/// resource file.  If MAX_LOADSTRING is 100, then it can hold at most 49
/// characters.
///
/// @internal The value of 128 is chosen for byte-alignment purposes
#define MAX_LOADSTRING    (128)


// Global Variables
HINSTANCE ghInst = NULL;                       ///< Current instance
WCHAR     gszTitle[ MAX_LOADSTRING ];          ///< The title bar text
WCHAR     gszWindowClass[ MAX_LOADSTRING ];    ///< The main window class name


// Forward declarations of private functions in this file
LRESULT CALLBACK WndProc( HWND, UINT, WPARAM, LPARAM );
INT_PTR CALLBACK About( HWND, UINT, WPARAM, LPARAM );


/// Program entrypoint
///
/// @see https://learn.microsoft.com/en-us/windows/win32/learnwin32/winmain--the-application-entry-point
///
int APIENTRY wWinMain(
   _In_     HINSTANCE hInstance,
   _In_opt_ HINSTANCE hPrevInstance,
   _In_     LPWSTR    lpCmdLine,
   _In_     int       nCmdShow ) {

   OutputDebugStringA( APP_NAME ": Starting" );

   BOOL    br;  // BOOL result
   HRESULT hr;  // HRESULT result

   ghInst = hInstance; /// Store the instance handle in a global variable

   /// Initialize COM (needs to be called once per each thread)
   hr = CoInitializeEx( NULL, COINIT_APARTMENTTHREADED );
   CHECK_HR( "Failed to initialize COM" )

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
   wcex.hIcon = LoadIcon( hInstance, MAKEINTRESOURCE( IDI_DTMF_DECODER ) );
   wcex.hCursor = LoadCursor( nullptr, IDC_ARROW );
   wcex.hbrBackground = NULL;  // To avoid flicker
   wcex.lpszMenuName = MAKEINTRESOURCEW( IDC_DTMFDECODER );
   wcex.lpszClassName = gszWindowClass;
   wcex.hIconSm = LoadIcon( wcex.hInstance, MAKEINTRESOURCE( IDI_DTMF_DECODER ) );

   if ( !RegisterClassExW( &wcex ) ) {
      OutputDebugStringA( APP_NAME ": Failed to register window class.  Exiting.");
      return FALSE;
   }

   /// Create the window
   ghMainWindow = CreateWindowW(
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

   if ( !ghMainWindow ) {
      OutputDebugStringA( APP_NAME ": Failed to create main window.  Exiting." );
      return FALSE;
   }

   /// Initialize the model
   if ( !mvcModelInit() ) {
      OutputDebugStringA( APP_NAME ": Failed to initialize the model.  Exiting." );
      return FALSE;
   }

   /// Initialize the view
   if( !mvcViewInitResources() ) {
      OutputDebugStringA( APP_NAME ": Failed to initialize the view.  Exiting." );
      return FALSE;
   }

   if ( !audioInit() ) {
      OutputDebugStringA( APP_NAME ": Failed to initialize the audio system.  Exiting." );
      return FALSE;
   }

   /// Initialize Win32-isms
   ShowWindow( ghMainWindow, nCmdShow );   // It's OK to ignore the result of this
   if ( !UpdateWindow( ghMainWindow ) ) {
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
   br = audioCleanup();
   CHECK_BR( "There was a problem cleaning up the audio resources.  Ending program." )

   CoUninitialize();  /// Unwind COM

   OutputDebugStringA( APP_NAME ": All global resources were cleaned up.  Ending program." );

   return (int) msg.wParam;
}


/// Message handler for the main window
///
/// @see https://learn.microsoft.com/en-us/windows/win32/api/winuser/nc-winuser-wndproc
///
LRESULT CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam ) {
   BOOL br;  // BOOL result

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
                  br = DestroyWindow( hWnd );
                  WARN_BR( "Failed to destroy window.  Investigate!!" );

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

            // Add any drawing code that uses hdc here...
            br = mvcViewPaintWindow();
            WARN_BR( "Failed to paint window.  Investigate!!" );

            br = EndPaint( hWnd, &ps );
            WARN_BR( "Failed to end paint.  Investigate!!" );
         }
         break;
      case WM_KEYDOWN:  /// WM_KEYDOWN - Exit if ESC is pressed
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
            br = goertzel_end();
            WARN_BR( "Failed to end the Goertzel DFT threads" );

            if ( gAudioSamplesReadyEvent != NULL ) {
               br = SetEvent( gAudioSamplesReadyEvent );
               WARN_BR( "Failed to signal gAudioSamplesReadyEvent" );
            }

            br = audioStopDevice();
            WARN_BR( "Failed to stop the audio device" );

            br = DestroyWindow( hWnd );
            WARN_BR( "Failed to destroy window" );

            break;
         }
      case WM_DESTROY:  /// WM_DESTROY - Post a quit message and return
         br = mvcViewCleanupResources();
         WARN_BR( "Failed to cleanup view resources" );

         br = goertzel_cleanup();
         WARN_BR( "Failed to cleanup Goertzel DFT" );

         PostQuitMessage( 0 );

         break;
      default:
         return DefWindowProc( hWnd, message, wParam, lParam );
   }
   return 0;
}


/// Message handler for the About dialog box
///
/// @see https://learn.microsoft.com/en-us/windows/win32/api/winuser/nc-winuser-wndproc
INT_PTR CALLBACK About( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam ) {
   BOOL br;  // BOOL result

   switch ( message ) {
      case WM_INITDIALOG:
         return (INT_PTR) TRUE;

      case WM_COMMAND:
         if ( LOWORD( wParam ) == IDOK || LOWORD( wParam ) == IDCANCEL ) {
            br = EndDialog( hDlg, LOWORD( wParam ) );
            WARN_BR( "Failed to end dialog in About" );
            return (INT_PTR) TRUE;
         }
         break;
   }
   return (INT_PTR) FALSE;
}
