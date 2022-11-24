///////////////////////////////////////////////////////////////////////////////
//          University of Hawaii, College of Engineering
//          DTMF_Decoder - EE 469 - Fall 2022
//
/// A Windows Desktop C program that decodes DTMF tones
///
/// The main loop (with wWinMain and WndProc)
///
/// @see https://learn.microsoft.com/en-us/windows/win32/
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
/// | `HIWORD`               | https://learn.microsoft.com/en-us/previous-versions/windows/desktop/legacy/ms632657(v=vs.85) |
///
/// @file    DTMF_Decoder.cpp
/// @author  Mark Nelson <marknels@hawaii.edu>
///////////////////////////////////////////////////////////////////////////////

#include "framework.h"    // Standard system include files
#include <combaseapi.h>   // For initializing COM
#include <pgobootrun.h>   // For PgoAutoSweep

#include "DTMF_Decoder.h" // For APP_NAME
#include "mvcModel.h"     // For the persistent model of the application
#include "mvcView.h"      // For drawing the window
#include "audio.h"        // For capturing audio
#include "goertzel.h"     // For goertzel_Stop()
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
/// @see https://learn.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-winmain
///
int APIENTRY wWinMain(
   _In_     HINSTANCE hInstance,     ///< Handle to this instance
   _In_opt_ HINSTANCE hPrevInstance, ///< Unused (legacy from Win16)
   _In_     LPWSTR    lpCmdLine,     ///< Command line arguments as a Unicode string
   _In_     int       nCmdShow )     ///< How the application window should be shown
   {
   // This is a test... but I'm going to keep it in for now.
   gracefulShutdown();            // This does not shutdown a program during init
// _ASSERTE( audioCleanup() );    // Can't call this before audioInit
   _ASSERTE( goertzel_Cleanup() );
// _ASSERTE( logCleanup() );      // Can't call this before logInit
   _ASSERTE( mvcModelCleanup() );
   _ASSERTE( mvcViewCleanup() );

   // The program really starts here

   _ASSERTE( hInstance != NULL );

   shInst = hInstance;  /// Store the instance handle in a global variable


   BOOL    br;  // BOOL result
   HRESULT hr;  // HRESULT result
   INT     ir;  // INT result

   /// Initialize the logger
   ///
   /// Tell the logger about where we hold the main window handle.  It's not
   /// set initially, but as soon as it is, the logger can start using it.
   br = logInit( &shInst, &ghMainWindow, APP_NAME, APP_NAME_W );
   if ( !br ) {
      LOG_FATAL( "Failed to initialize the logger.  Exiting." );
      return EXIT_FAILURE;
   }

   LOG_TRACE_R( IDS_DTMF_DECODER_STARTING, APP_NAME_W );  // "Starting DTMF Decoder"


   /// Set #gbIsRunning to `true`.  Set it to `false` if we need to shutdown.
   /// For example, #gbIsRunning gets set to false by WM_CLOSE.
   /// Remember:  #gbIsRunning is a `bool`, so use `false` not `FALSE`.
   gbIsRunning = true;

   /// Initialize COM (needs to be called once per each thread)
   hr = CoInitializeEx( NULL, COINIT_APARTMENTTHREADED );
   if ( FAILED( hr ) ) {
      LOG_FATAL_R( IDS_DTMF_DECODER_FAILED_TO_INITIALIZE_COM );  // "Failed to initialize COM.  Exiting."
      return EXIT_FAILURE;
   }

   /// Initialize global strings
   ir = LoadStringW( hInstance, IDS_APP_TITLE, sswTitle, MAX_LOADSTRING );
   if ( !ir ) {
      LOG_FATAL_R( IDS_DTMF_DECODER_FAILED_TO_RETRIEVE_TITLE );  // "Failed to retrive app title.  Exiting."
      CoUninitialize();       // Unwind COM
      return EXIT_FAILURE;
   }

   ir = LoadStringW( hInstance, IDC_DTMFDECODER, sswWindowClass, MAX_LOADSTRING );
   if ( !ir ) {
      LOG_FATAL_R( IDS_DTMF_DECODER_FAILED_TO_RETRIEVE_CLASS_NAME );  // "Failed to retrieve window class name.  Exiting."
      CoUninitialize();       // Unwind COM
      return EXIT_FAILURE;
   }

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
      LOG_FATAL_R( IDS_DTMF_DECODER_FAILED_TO_REGISTER_WINDOW_CLASS );  // "Failed to register window class.  Exiting."
      return EXIT_FAILURE;
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
      LOG_FATAL_R( IDS_DTMF_DECODER_FAILED_TO_CREATE_MAIN_WINDOW );  // "Failed to create main window.  Exiting."
      CoUninitialize();       // Unwind COM
      return EXIT_FAILURE;
   }

   LOG_TRACE_R( IDS_DTMF_DECODER_CREATED_MAIN_WINDOW, giWindowWidth, giWindowHeight );  // "Created main window:  Width=%d  Height=%d"


   /// Register a custom message that the worker threads can use to report
   /// problems back to the application's main thread.

   //  GUID generated by GUIDGEN
   guUMW_CLOSE_FATAL = RegisterWindowMessageA( APP_NAME " 9B66F531-03F2-4FE7-9D9A-7D09122EBE88" );
   if ( guUMW_CLOSE_FATAL == 0 ) {
      LOG_FATAL_R( IDS_DTMF_DECODER_FAILED_TO_REGISTER_CUSTOM_MESSAGE );  // "Failed to register a custom message.  Exiting."
      CoUninitialize();       // Unwind COM
      return EXIT_FAILURE;
   }

   /// Initialize the model
   br = mvcModelInit();
   if ( !br ) {
      LOG_FATAL_R( IDS_DTMF_DECODER_FAILED_TO_INITIALIZE_MODEL );  // "Failed to initialize the model.  Exiting."
      CoUninitialize();       // Unwind COM
      return EXIT_FAILURE;
   }

   /// Initialize the view
   br = mvcViewInit();
   if ( !br ) {
      LOG_FATAL_R( IDS_DTMF_DECODER_FAILED_TO_INITIALIZE_VIEW );  // "Failed to initialize the view.  Exiting."
      mvcModelCleanup();      // Unwind mvcViewInit
      CoUninitialize();       // Unwind COM
      return EXIT_FAILURE;
   }

   /// Initialize the audio capture device & thread
   br = audioInit();
   if ( !br ) {
      LOG_FATAL_R( IDS_DTMF_DECODER_FAILED_TO_INITIALIZE_AUDIO );  // "Failed to initialize the audio capture system.  Exiting."
      mvcViewCleanup();
      mvcModelCleanup();
      CoUninitialize();       // Unwind COM
      return EXIT_FAILURE;
   }

   HACCEL hAccelTable = LoadAccelerators( hInstance, MAKEINTRESOURCE( IDC_DTMFDECODER ) );
   if ( hAccelTable == NULL ) {
      LOG_FATAL_R( IDS_DTMF_DECODER_FAILED_TO_LOAD_MENU );  // "Failed to load menu accelerator.  Exiting."
      mvcViewCleanup();
      mvcModelCleanup();
      CoUninitialize();       // Unwind COM
      audioCleanup();
      return EXIT_FAILURE;
   }

   ShowWindow( ghMainWindow, nCmdShow );   // It's OK to ignore the result of ShowWindow

   LOG_INFO_R( IDS_DTMF_DECODER_APP_RESOURCES_READY );  // "All application resources were successfully initialized"

   /// The application's message loop
   MSG msg;
   ZeroMemory( &msg, sizeof( msg ) );  // Returns void, so nothing to check
   BOOL bRet;

   while ( ( bRet = GetMessage( &msg,  // The message structure
                                NULL,  // Get messages for any window that belongs to the current thread
                                0, 0   // Retrieve all available messages
                              ) ) != 0 ) {
      if ( bRet > 0 ) {
         if ( !TranslateAccelerator( msg.hwnd, hAccelTable, &msg ) ) {
            TranslateMessage( &msg );
            DispatchMessage( &msg );
         }
      } else if ( bRet == 0 ) {
         gracefulShutdown();  // This is a normal exit
      } else {  // bRet < 0
         // There's a risk of an infinite loop here.  If that happens, then
         // change this to #logSetMsg and break out of the loop.
         CLOSE_FATAL( IDS_DTMF_DECODER_FAILED_TO_GET_MESSAGE, 0 );  // "Failed to get a message.  Ending program."
      }
   }

   br = goertzel_Stop();
   WARN_BR_R( IDS_DTMF_DECODER_FAILED_TO_END_DFT_THREADS );  // "Failed to end the Goertzel DFT threads"

   /// After all of the threads have stopped, see if they have any messages
   /// to report
   if ( logQueueHasEntry() ) {
      /// @todo This
   }

   /// Cleanup all resources in the reverse order they were created
   br = audioCleanup();
   WARN_BR_R( IDS_DTMF_DECODER_FAILED_TO_CLEANUP_AUDIO );  // "Failed to clean up audio resources."

   // The view was cleaned up in WM_DESTRO (immediately following the destruction
   // of the window

   br = goertzel_Cleanup();
   WARN_BR_R( IDS_DTMF_DECODER_FAILED_TO_CLEANUP_DFT );  // "Failed to cleanup Goertzel DFT"

   br = mvcModelCleanup();
   WARN_BR_R( IDS_DTMF_DECODER_FAILED_TO_CLEANUP_MODEL );  // "Failed to cleanup the model."

   // The window has already been cleaned up

   // Don't un-register the windows class because:
   //   1. There may be other instances of DTMF Decoder running
   //   2. Windows will clean this up on its own

   CoUninitialize();         // Unwind COM

   br = logCleanup();
   WARN_BR_R( IDS_DTMF_DECODER_FAILED_TO_CLEANUP_LOGS );  // "Failed to cleanup the logs."

   // The log can still be called even after it's been "cleaned up"
   if ( giApplicationReturnValue == EXIT_SUCCESS ) {
      LOG_INFO_R( IDS_DTMF_DECODER_ENDING_SUCCESSFULLY, APP_NAME_W );  // "All %s resources were cleaned up.  Ending program:  SUCCESS."
   } else {
      LOG_INFO_R( IDS_DTMF_DECODER_ENDING_IN_FAILURE_MODE, APP_NAME_W );  // "All %s resources were cleaned up.  Ending program in failure mode."
   }

   return giApplicationReturnValue;
}


/// Message handler for the main window
///
/// @see https://learn.microsoft.com/en-us/windows/win32/api/winuser/nc-winuser-wndproc
///
LRESULT CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam ) {
   BOOL br;  // BOOL result

   switch ( message ) {
      case WM_COMMAND: { /// WM_COMMAND - Process the application menu
            int wmId = LOWORD( wParam );
            // Parse the menu selections
            switch ( wmId ) {
               case IDM_ABOUT:
                  DialogBox( shInst, MAKEINTRESOURCE( IDD_ABOUTBOX ), hWnd, About );
                  // DialogBox returns void -- nothing to check
                  break;
               case IDM_EXIT:
                  gracefulShutdown();
                  break;
               default:
                  return DefWindowProc( hWnd, message, wParam, lParam );
            }
         }
         break;
      case WM_PAINT: { /// WM_PAINT - Paint the main window
            RECT updateRect;
            br = GetUpdateRect( hWnd, &updateRect, FALSE );
            if ( !br ) {
               break;  /// If there is no update region, then don't paint anything
            }

            PAINTSTRUCT ps;
            HDC hdc = BeginPaint( hWnd, &ps );
            (void) hdc;  // Suppress a compiler warning that hdc
                         // is not checked after this.  No code is generated.

            // Paint the window
            br = mvcViewPaintWindow( &updateRect );
            CHECK_BR_C( IDS_DTMF_DECODER_FAILED_TO_PAINT, 0 );  // "Failed to paint window.  Investigate!!"

            br = EndPaint( hWnd, &ps );
            CHECK_BR_C( IDS_DTMF_DECODER_FAILED_TO_END_PAINT, 0 );  // "Failed to end paint.  Investigate!!"
         }
         break;
      case WM_KEYDOWN:  /// WM_KEYDOWN - Exit if ESC is pressed
         {
            switch ( wParam ) {
               case VK_ESCAPE:  /// Exit the app (normally) when ESC is pressed
                  // logTest();      // This is a good place to test the logger
                  gracefulShutdown();
                  break ;
               default:
                  break ;
            }
         }
         break ;
      case WM_CLOSE:    /// WM_CLOSE - Start the process of closing the application
         {
            gbIsRunning = false;

            if ( ghAudioSamplesReadyEvent != NULL ) {
               br = SetEvent( ghAudioSamplesReadyEvent );
               WARN_BR_R( IDS_DTMF_DECODER_FAILED_TO_SIGNAL_gAUDIOSAMPLESREADY );  // "Failed to signal gAudioSamplesReadyEvent"
            }

            br = audioStopDevice();
            WARN_BR_R( IDS_DTMF_DECODER_FAILED_TO_STOP_AUDIO_DEVICE );  // "Failed to stop the audio device"

            br = DestroyWindow( hWnd );
            if ( !br ) {
               // At this point, the guUMW_CLOSE_FATAL message never makes it
               // through the message loop, so we need to set the message directly
               logSetMsg( LOG_LEVEL_FATAL, IDS_DTMF_DECODER_FAILED_TO_DESTROY_WINDOW, 0 );
               LOG_FATAL_Q( IDS_DTMF_DECODER_FAILED_TO_DESTROY_WINDOW );
            }
            break;
         }
      case WM_DESTROY:  /// WM_DESTROY - Post a quit message
         ghMainWindow = NULL;

         br = mvcViewCleanup();
         WARN_BR_R( IDS_DTMF_DECODER_FAILED_TO_CLEANUP_VIEW );  // "Failed to cleanup view resources"

         PostQuitMessage( giApplicationReturnValue );

         break;
      default:                                  // Because guUMW_CLOSE_FATAL is a variable
         if ( message == guUMW_CLOSE_FATAL ) {  // it can't be tested in a switch statement
            giApplicationReturnValue = EXIT_FAILURE;

            WORD resource_id = LOWORD( wParam );

            logSetMsg( LOG_LEVEL_FATAL, resource_id, wParam );

            gracefulShutdown();
            break;
         }
         return DefWindowProcW( hWnd, message, wParam, lParam );  // Can't check return value
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
/// This does not shutdown the program **before** the message loop starts
///
/// This is both a normal and failure-mode shutdown, so this doesn't set
/// #giApplicationReturnValue
///
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
         WARN_BR_R( IDS_DTMF_DECODER_ABOUT_FAILED_TO_SET_NAME );  // "Failed to set the name and version of the app"

         br = SetDlgItemTextA( hDlg, IDC_DATE, "Built: " __DATE__ "" );
         /// @todo The date needs to be localized
         WARN_BR_R( IDS_DTMF_DECODER_ABOUT_FAILED_TO_SET_DATE );  // "Failed to set the build date of the app"

         return (INT_PTR) TRUE;

      case WM_COMMAND:
         if ( LOWORD( wParam ) == IDOK || LOWORD( wParam ) == IDCANCEL ) {
            br = EndDialog( hDlg, LOWORD( wParam ) );
            WARN_BR_R( IDS_DTMF_DECODER_ABOUT_FAILED_TO_END );  // "Failed to end the About window"
            return (INT_PTR) TRUE;
         }
         break;
   }
   return (INT_PTR) FALSE;
}
