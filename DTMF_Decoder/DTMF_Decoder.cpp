///////////////////////////////////////////////////////////////////////////////
//          University of Hawaii, College of Engineering
//          DTMF_Decoder - EE 469 - Fall 2022
//
/// A Windows Desktop C program that decodes DTMF tones
///
/// The main loop (with wWinMain and WndProc)
///
/// @see https://learn.microsoft.com/en-us/windows/win32/learnwin32/winmain--the-application-entry-point
/// @see https://learn.microsoft.com/en-us/windows/win32/api/winuser/nc-winuser-wndproc
/// @see https://learn.microsoft.com/en-us/windows/win32/winmsg/using-messages-and-message-queues
///
/// ## APIs Used
/// | API                    | Link                                                                                         |
/// |------------------------|----------------------------------------------------------------------------------------------|
/// | `CoInitializeEx`       | https://learn.microsoft.com/en-us/windows/win32/api/combaseapi/nf-combaseapi-coinitializeex  |
/// | `CoUninitialize`       | https://learn.microsoft.com/en-us/windows/win32/api/combaseapi/nf-combaseapi-couninitialize  |
/// | `LoadStringW`          | https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-loadstringw           |
/// | `CreateWindowW`        | https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-createwindoww         |
/// | `RegisterClassExW`     | https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-registerclassexw      |
/// | `ShowWindow`           | https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-showwindow            |
/// | `UpdateWindow`         | https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-updatewindow          |
/// | `DefWindowProc`        | https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-defwindowproca        |
/// | `GetMessage`           | https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getmessage            |
/// | `DispatchMessage`      | https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-dispatchmessage       |
/// | `LoadAccelerators`     | https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-loadacceleratorsa     |
/// | `MAKEINTRESOURCE`      | https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-makeintresourcea      |
/// | `PostMessageA`         | https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-postmessagea          |
/// | `TranslateAccelerator` | https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-translateacceleratora |
/// | `TranslateMessage`     | https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-translatemessage      |
/// | `DialogBox`            | https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-dialogboxa            |
/// | `DestroyWindow`        | https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-destroywindow         |
/// | `PostQuitMessage`      | https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-postquitmessage       |
/// | `EndDialog`            | https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-enddialog             |
/// | `BeginPaint` (GDI)     | https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-beginpaint            |
/// | `EndPaint` (GDI)       | https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-endpaint              |
/// | `SecureZeroMemory`     | https://learn.microsoft.com/en-us/previous-versions/windows/desktop/legacy/aa366877(v=vs.85) |
/// | `SetEvent`             | https://learn.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-setevent            |
/// | `SetDlgItemTextA`      | https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-setdlgitemtexta       |
///
/// @file    DTMF_Decoder.cpp
/// @author  Mark Nelson <marknels@hawaii.edu>
///////////////////////////////////////////////////////////////////////////////

#include "framework.h"    // Standard system include files
#include <combaseapi.h>   // For initializing COM
#include <pgobootrun.h>   // For PgoAutoSweep

#include "DTMF_Decoder.h" // For APP_NAME
#include "mvcModel.h"     // Holds the persistent model for the application
#include "mvcView.h"      // For drawing the window
#include "audio.h"        // For capturing audio
#include "goertzel.h"     // For goertzel_end()
#include "resource.h"     // For the resource definitions
#include "version.h"      // For the application's version strings


/// Defines the size of the wide-string buffer used to get strings from the
/// resource file.  If MAX_LOADSTRING is 100, then it can hold at most 49
/// characters.
///
/// @internal The value of 128 is chosen for byte-alignment purposes
#define MAX_LOADSTRING    (128)


// Global Variables
static HINSTANCE shInst = NULL;                       ///< Current instance
static WCHAR     sswTitle[ MAX_LOADSTRING ];          ///< The title bar text
static WCHAR     sswWindowClass[ MAX_LOADSTRING ];    ///< The main window class name


// Forward declarations of private functions in this file
LRESULT CALLBACK WndProc( HWND, UINT, WPARAM, LPARAM );
INT_PTR CALLBACK About( HWND, UINT, WPARAM, LPARAM );


/// Program entrypoint
///
/// @see https://learn.microsoft.com/en-us/windows/win32/learnwin32/winmain--the-application-entry-point
///
int APIENTRY wWinMain(
   _In_     HINSTANCE hInstance,     // Handle to this instance
   _In_opt_ HINSTANCE hPrevInstance, // Unused (legacy from Win16)
   _In_     LPWSTR    lpCmdLine,     // Command line arguments as a Unicode string
   _In_     int       nCmdShow ) {   // How the application window should be shown


   gracefulShutdown();  // This does not shutdown a program during init
   LOG_TRACE( "audioCleanup returned %d", audioCleanup() );
   LOG_TRACE( "goertzel_cleanup returned %d", goertzel_cleanup() );
   LOG_TRACE( "logCleanup returned %d", logCleanup() );
   LOG_TRACE( "mvcModelCleanup returned %d", mvcModelCleanup() );
   LOG_TRACE( "mvcViewCleanup returned %d", mvcViewCleanup() );


   _ASSERTE( hInstance != NULL );

   shInst = hInstance; /// Store the instance handle in a global variable

   BOOL    br;  // BOOL result
   HRESULT hr;  // HRESULT result
   INT     ir;  // INT result

   /// Initialize the logger
   ///
   /// Tell the logger about where we hold the main window handle.  It's not
   /// set initially, but as soon as it is, the logger can start using it.
   br = logInit( &ghMainWindow );
   CHECK_BR( "Failed to initialize the logger.  Exiting." );

   LOG_TRACE( "Starting" );

   /// Set #gbIsRunning to `true`.  Set it to `false` if we need to shutdown.
   /// For example, #gbIsRunning gets set to false by WM_CLOSE.
   /// Remember:  #gbIsRunning is a `bool`, so use `false` not `FALSE`.
   gbIsRunning = true;

   /// Initialize COM (needs to be called once per each thread)
   hr = CoInitializeEx( NULL, COINIT_APARTMENTTHREADED );
   CHECK_HR( "Failed to initialize COM" )

   /// Initialize global strings
   ir = LoadStringW( hInstance, IDS_APP_TITLE,   sswTitle,       MAX_LOADSTRING );
   CHECK_IR( "Failed to retrive app title" );

   ir = LoadStringW( hInstance, IDC_DTMFDECODER, sswWindowClass, MAX_LOADSTRING );
   CHECK_IR( "Failed to retrieve window class name" );

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
   wcex.lpszClassName = sswWindowClass;
   wcex.hIconSm = LoadIcon( wcex.hInstance, MAKEINTRESOURCE( IDI_DTMF_DECODER ) );

   if ( !RegisterClassExW( &wcex ) ) {
      LOG_FATAL( "Failed to register window class.  Exiting.");
      return FALSE;
   }

   /// Create the window
   ghMainWindow = CreateWindowW(
      sswWindowClass,
      sswTitle,
      WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
      CW_USEDEFAULT,          // X position of window
      0,                      // Y position of window
      giWindowWidth,          // Width of window
      giWindowHeight,         // Height of window
      nullptr,                // hWndParent
      nullptr,                // hMenu
      hInstance,              // hInstance
      nullptr );              // lpParam

   if ( !ghMainWindow ) {
      LOG_FATAL( "Failed to create main window.  Exiting." );
      return FALSE;
   }

   LOG_TRACE( "Created main window:  Width=%d  Height=%d", giWindowWidth, giWindowHeight );

   /// Initialize the model
   br = mvcModelInit();
   CHECK_BR( "Failed to initialize the model.  Exiting." );

   /// Initialize the view
   br = mvcViewInit();
   CHECK_BR( "Failed to initialize the view.  Exiting." );

   /// Initialize the audio capture device & thread
   br = audioInit();
   CHECK_BR( "Failed to initialize the audio system.  Exiting." );

   /// Initialize Win32 message loop
   ShowWindow( ghMainWindow, nCmdShow );   // It's OK to ignore the result of this

   HACCEL hAccelTable = LoadAccelerators( hInstance, MAKEINTRESOURCE( IDC_DTMFDECODER ) );
   if ( hAccelTable == NULL ) {
      LOG_FATAL( "Failed to load manu accelerator.  Exiting." );
      return FALSE;
   }

   LOG_INFO( "All global resources were successfully initialized" );


   /// Main message loop
   MSG msg;
   ZeroMemory( &msg, sizeof( msg ) );
   BOOL bRet;

   while ( ( bRet = GetMessage( &msg,  // The message structure
                                NULL,  // Get messages for any window that belongs to the current thread
                                0, 0   // Retrieve all available messages
                              ) ) != 0 ) {
      if ( bRet <= 0 ) {
         if ( bRet <= -1 ) {
            LOG_FATAL( "Failed to get a message.  Ending program." );
         }
         gracefulShutdown();
      }

      if ( !TranslateAccelerator( msg.hwnd, hAccelTable, &msg ) ) {
         TranslateMessage( &msg );
         DispatchMessage( &msg );
      }
   }

   /// Cleanup all resources
   br = audioCleanup();
   WARN_BR( "There was a problem cleaning up the audio resources.  Ending program." )

   CoUninitialize();  /// Unwind COM

   LOG_INFO( "All global resources were cleaned up.  Ending program." );

   br = logCleanup();
   WARN_BR( "Failed to cleanup the logs." );

   return (int) msg.wParam;  /// The return value `msg.pParam` comes to us
                             /// from `WM_QUIT` which gets set by PostQuitMessage
                             /// which gets its value from  #giApplicationReturnValue.
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
                  DialogBox( shInst, MAKEINTRESOURCE( IDD_ABOUTBOX ), hWnd, About );
                  // DialogBox returns void -- no checking required
                  break;
               case IDM_EXIT:
                  gracefulShutdown();
                  break;
               default:
                  return DefWindowProc( hWnd, message, wParam, lParam );
            }
         }
         break;
      case WM_PAINT:  /// WM_PAINT - Paint the main window
         {
            RECT updateRect;
            br = GetUpdateRect( hWnd, &updateRect, FALSE );

            if ( !br ) {
               break;  // If there is no update region, then don't paint anything
            }

            PAINTSTRUCT ps;
            HDC hdc = BeginPaint( hWnd, &ps );

            (void) hdc;  // Suppress a compiler warning that hdc
                         // is not checked after this.  No code is generated.

            // Add any drawing code here...
            br = mvcViewPaintWindow( &updateRect );
            WARN_BR( "Failed to paint window.  Investigate!!" );

            br = EndPaint( hWnd, &ps );
            WARN_BR( "Failed to end paint.  Investigate!!" );
         }
         break;
      case WM_KEYDOWN:  /// WM_KEYDOWN - Exit if ESC is pressed
         {
            switch ( wParam ) {
               case VK_ESCAPE:
                  // logTest();      // This is a good place to test the logger
                  PostMessageA( hWnd, WM_CLOSE, 0, 0 );
                  break ;
               default:
                  break ;
            }
         }
         break ;
      case WM_CLOSE:    /// WM_CLOSE - Start the process of closing the application
         {
            gbIsRunning = false;
            br = goertzel_end();
            WARN_BR( "Failed to end the Goertzel DFT threads" );

            if ( ghAudioSamplesReadyEvent != NULL ) {
               br = SetEvent( ghAudioSamplesReadyEvent );
               WARN_BR( "Failed to signal gAudioSamplesReadyEvent" );
            }

            br = audioStopDevice();
            WARN_BR( "Failed to stop the audio device" );

            br = DestroyWindow( hWnd );
            WARN_BR( "Failed to destroy window" );

            break;
         }
      case WM_DESTROY:  /// WM_DESTROY - Post a quit message and return
         br = mvcViewCleanup();
         WARN_BR( "Failed to cleanup view resources" );

         br = goertzel_cleanup();
         WARN_BR( "Failed to cleanup Goertzel DFT" );

         PostQuitMessage( giApplicationReturnValue );

         break;
      default:
         return DefWindowProc( hWnd, message, wParam, lParam );
   }
   return 0;
}


/// Gracefully initiate the shutdown of the application
///
/// The app has multiple threads and message loops, so #gracefulShutdown has to
/// do things like:
///   - Tell the thread loops to quit
///   - Signal the callback handles
///   - Actually drop out of the thread loops
///   - Cleanup the resources in use
///
/// This function doesn't do these things, but it gets the ball rolling by
/// effectively pressing the Close button on the window.
///
/// NOTE:  This does not shutdown the program **before** the message loop starts.
void gracefulShutdown() {
   gbIsRunning = false;
   PostMessageA( ghMainWindow, WM_CLOSE, 0, 0 );  // Shutdown the app
}


/// Message handler for the About dialog box
///
/// @see https://learn.microsoft.com/en-us/windows/win32/api/winuser/nc-winuser-wndproc
///
INT_PTR CALLBACK About( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam ) {
   BOOL br;  // BOOL result

   switch ( message ) {
      case WM_INITDIALOG:
         br = SetDlgItemTextA( hDlg, IDC_PROGRAM_NAME, APP_NAME ", " FULL_VERSION );
         CHECK_BR( "Failed to set the name and version of the app")

         br = SetDlgItemTextA( hDlg, IDC_DATE, "Built: " __DATE__ "" );
         CHECK_BR( "Failed to set the build date of the app" );

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
