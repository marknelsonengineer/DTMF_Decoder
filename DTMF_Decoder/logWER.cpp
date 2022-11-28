///////////////////////////////////////////////////////////////////////////////
//          University of Hawaii, College of Engineering
//          DTMF_Decoder - EE 469 - Fall 2022
//
//  A Windows Desktop C program that decodes DTMF tones
//
/// Generic Windows Error Reporting utility
///
/// This utility is 'bolted' onto the logger log.cpp.  If it's enabled (through
/// `#ifdef`s) it will generate a Windows Error Report if the application
/// detects a fatal error.
///
/// The data logWER collects is:
///   - The first 10 error codes (the numeric error from the string table)
///   - The first 10 error message
///   - The first 4K of log messages
///   - The last 4K of log messages
///
/// The design is such that the main application doesn't need to know much
/// about logWER.
///
/// Applications need to be digitally signed before they can be recorded in
/// the live WER database... where developers can get access to the analytics.
///
/// Users should see the logger's `MessageBox`-based messages but not the WER
/// dialog box.
///
/// @see https://learn.microsoft.com/en-us/windows/win32/wer/windows-error-reporting
/// @see Windows Quality Online Services
/// @see https://en.wikipedia.org/wiki/Winqual
///
/// @todo:  Complete this bit
/// ## Generic Win32 API
/// | API                 | Link                                                                                                                              |
/// |---------------------| ----------------------------------------------------------------------------------------------------------------------------------|
/// | `LoadStringW`       | https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-loadstringw                                                |
/// GetProcessImageFileNameW
///
/// ## Debugging& Instrumentation API
/// | API | Link                                                                                                                                               |
/// |----------------------| ----------------------------------------------------------------------------------------------------------------------------------|
/// | `va_arg`             | https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/va-arg-va-copy-va-end-va-start?view=msvc-170                    |
///
/// ## CRT & Memory Management API
/// | API                | Link                                                                                         |
/// |--------------------| ---------------------------------------------------------------------------------------------|
/// | `CopyMemory`       | https://learn.microsoft.com/en-us/previous-versions/windows/desktop/legacy/aa366535(v=vs.85) |
///
/// @file    logWER.cpp
/// @author  Mark Nelson <marknels@hawaii.edu>
///////////////////////////////////////////////////////////////////////////////

#include "framework.h"    // Standard system include files
#include "logWER.h"       // For yourself

#include <werapi.h>       // For WerGetFlags
#include "log.h"          // For LOG_INFO
#include "log_ex.h"       // For extensions to the log

#include <inttypes.h>     // For PRIu16
#include <psapi.h>        // For GetProcessImageFileNameW
#include <strsafe.h>      // For StringCchPrintf

#pragma comment(lib, "Wer")  // Link the WER library


HREPORT hReport = NULL;                         ///< Handle to the windows error report
WCHAR   wzFullExeFilename[ MAX_PATH ] = { 0 };  ///< Full path to the executable file
WCHAR   reportName[ 64 ] = { 0 };               ///< Name of the report



///////////////////////////////////////////////////////////////////////////////
// End of declarations from log.cpp


/// Initialize Windows Error Reporting
///
/// @param phInst   The application instance handle.  **This must be a global
///                 variable.**
///
/// @param phWindow The window that will own the log message boxes (usually the
///                 application's main window).  **This must be a global variable.**
///                 It is OK to set this to `NULL`
///
/// @param pAppName Save the name of the application so we don't have to pass
///                 it every time we log something.  It's OK to set this to
///                 `NULL` or an empty string.  This is used as the title for
///                 `MessageBoxA`.
///
/// @param pwAppName Same as pAppName but for wide characters and `MessageBoxW`
///
/// @return `TRUE` if successful.  `FALSE` if there was a problem.
BOOL logWerInit(
   _In_         HINSTANCE* phInstance,
   _In_         HWND*      phWindow,
   _In_z_ const CHAR*      pAppName,
   _In_z_ const WCHAR*     pwAppName
   ) {
   /// The parameters are checked (validated) by #logValidate via #logInit
   //  ...but we'll do some ASSERT checking just to be sure
   _ASSERTE( phInstance != NULL );
   _ASSERTE( phWindow   != NULL );
   _ASSERTE( pAppName   != NULL );
   _ASSERTE( pwAppName  != NULL );

   DWORD   dwr;  // DWORD result
   HRESULT hr;   // HRESULT result

   HANDLE hProcess = GetCurrentProcess();
   // No error checking available

   dwr = GetProcessImageFileNameW( hProcess, wzFullExeFilename, sizeof( wzFullExeFilename ) );
   if ( dwr <= 0 ) {
      RETURN_FATAL_R( IDS_LOG_WER_FAILED_TO_GET_EXE_PATH );  // "Unable to get the executable's full path name"
   }
   LOG_INFO_R( IDS_LOG_WER_FULL_EXE_FILENAME, wzFullExeFilename );  // "GetProcessImageFileNameW Full Exe Filename=%s"


   WER_REPORT_INFORMATION myReport = { 0 };

   myReport.dwSize = sizeof( myReport );
   myReport.hProcess = NULL;  // Handle to the report's process, if `NULL` it's the calling process
// myReport.wzConsentKey defaults to pwzEventType, which is fine
// myReport.wzFriendlyEventName defaults to pwzEventType, which is fine
   CopyMemory( myReport.wzApplicationName, pwAppName, sizeof( myReport.wzApplicationName ) );
   CopyMemory( myReport.wzApplicationPath, wzFullExeFilename, sizeof( myReport.wzApplicationPath ) );

   wBuffer_t format = { L"", BUFFER_GUARD };
   logGetStringFromResources( IDS_LOG_WER_DESCRIPTION, &format );  // "%s has a problem and needs to shutdown.  A report will be generated and sent to the developer."
   StringCchPrintfW( myReport.wzDescription, sizeof( myReport.wzDescription ), format.sBuf, pwAppName );

   myReport.hwndParent = *phWindow;


   format = { L"", BUFFER_GUARD };

   StringCchPrintfW( reportName, sizeof( reportName ), L"%s Error Report", pwAppName );

   hr = WerReportCreate(
      reportName,  // A Unicode string with the name of this event
      WerReportCritical,   // The type of report
      &myReport,           // A pointer to a WER_REPORT_INFORMATION structure
      &hReport );          // A handle to the report. If the function fails, this handle is NULL.

   LOG_INFO( "WerReportCreate hr=%ld  hReport=%zu", hr, hReport );


   LOG_INFO( "Done WER" );

   return TRUE;
}


BOOL logWerEvent() {
   HRESULT hr;  // HRESULT result

   hr = WerReportSetParameter(
      hReport,  // handle to the report
      WER_P0,  // identifier of the parameter to be set
      L"Parm name 0",  //Unicode string that contains the name of the parameter
      L"Lorem ipsum"   // The parameter value.
   );

   LOG_INFO( "WerReportSetParameter hr=%ld", hr );

   return TRUE;
}


/// Submit a Windows Error Report.  This will display a modal dialog box.
///
/// @return `TRUE` if successful.  `FALSE` if there was a problem.
BOOL logWerSubmit() {
   HRESULT hr;  // HRESULT result


//   hr = WerReportAddDump(
//      hReport,  // handle to the report
//      GetCurrentProcess(),
//      GetCurrentThread(),
//      WerDumpTypeMiniDump,
//      NULL,
//      NULL,
//      0 );
//
//   LOG_INFO( "WerReportAddDump hr=%ld", hr );



   WER_SUBMIT_RESULT submissionResult;

   hr = WerReportSubmit(
      hReport,            // The report handle
      WerConsentApproved, // The consent status enum  // WerConsentAlwaysPrompt   WerConsentNotAsked  WerConsentApproved
      WER_SUBMIT_NO_QUEUE | WER_SUBMIT_OUTOFPROCESS | WER_SUBMIT_SHOW_DEBUG | WER_SUBMIT_REPORT_MACHINE_ID,                  // Submission flags:  WER_SUBMIT_QUEUE | WER_SUBMIT_OUTOFPROCESS
      &submissionResult
   );
   CHECK_HR_R( IDS_LOG_WER_FAILED_TO_SUBMIT_REPORT );  // "Unable to submit Windows Error Report"

   LOG_TRACE( "WER Report submitted successfully" );

   return TRUE;
}


/// Cleanup Windows Error Reporting resources
///
/// @return `TRUE` if successful.  `FALSE` if there was a problem.
BOOL logWerCleanup() {
   HRESULT hr;

   if ( hReport != NULL ) {
      hr = WerReportCloseHandle( hReport );
      CHECK_HR_R( IDS_LOG_WER_FAILED_TO_CLOSE_HANDLE );  // "Failed to close WER handle"

      hReport = NULL;
   }

   return TRUE;
}
