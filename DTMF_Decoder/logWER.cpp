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
#include "version.h"      // For FULL_VERSION_W

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
/// As an extension to log.cpp, WER can see all of the variables set in
/// #logInit.
///
/// @return `TRUE` if successful.  `FALSE` if there was a problem.
BOOL logWerInit() {
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
   CopyMemory( myReport.wzConsentKey,        swAppName,         sizeof( myReport.wzConsentKey        ) );
   CopyMemory( myReport.wzFriendlyEventName, swAppTitle,        sizeof( myReport.wzFriendlyEventName ) );
   CopyMemory( myReport.wzApplicationName,   swAppTitle,        sizeof( myReport.wzApplicationName   ) );
   CopyMemory( myReport.wzApplicationPath,   wzFullExeFilename, sizeof( myReport.wzApplicationPath   ) );

   // Compose myReport.wzDescription
   wBuffer_t format = { L"", BUFFER_GUARD };
   logGetStringFromResources( IDS_LOG_WER_DESCRIPTION, &format );  // "%s has a problem and needs to shutdown.  A report will be generated and sent to the developer."
   StringCchPrintfW( myReport.wzDescription, sizeof( myReport.wzDescription ), format.sBuf, swAppTitle );

   myReport.hwndParent = *sphMainWindow;

   // Compose reportName
   format = { L"", BUFFER_GUARD };
   logGetStringFromResources( IDS_LOG_WER_REPORT_NAME, &format );  // "%s Error Report"
   StringCchPrintfW( reportName, sizeof( reportName ), format.sBuf, swAppTitle );

   hr = WerReportCreate(
      reportName,          // A Unicode string with the name of this event
      WerReportCritical,   // The type of report
      &myReport,           // A pointer to a WER_REPORT_INFORMATION structure
      &hReport );          // A handle to the report. If the function fails, this handle is NULL.

   if ( hr != S_OK || hReport == NULL ) {
      LOG_WARN_R( IDS_LOG_WER_FAILED_CREATE_REPORT );  // "Failed to create a Windows Error Report (should it be needed later).  Continuing."
   }

   LOG_INFO_R( IDS_LOG_WER_INIT_SUCCESS );  // "Initialized Windows Error Reporting."

   return TRUE;
}


BOOL logWerEvent(
   _In_   const logLevels_t logLevel,
   _In_z_ const WCHAR*      resourceName,
   _In_   const UINT        resourceId,
   _In_z_ const WCHAR*      logMsg
   ) {
   _ASSERTE( resourceName != NULL );
   _ASSERTE( resourceName[ 0 ] != L'\0' );
   _ASSERTE( logMsg != NULL );
   _ASSERTE( logMsg[ 0 ] != L'\0' );

   if ( logLevel < LOG_LEVEL_ERROR ) {  /// Don't set any WER parameters if the
      return TRUE;                      /// log level is less than #LOG_LEVEL_ERROR
   }

   HRESULT hr;  // HRESULT result

   hr = WerReportSetParameter(
      hReport,       // Handle to the report
      WER_P0,        // Identifier of the parameter to be set
      L"Log Level",  // Unicode string that contains the name of the parameter
      ( logLevel == LOG_LEVEL_ERROR ) ? L"ERROR" : L"FATAL"   // The parameter value
   );
   WARN_HR_R( IDS_LOG_WER_FAILED_TO_SET_PARAMETER );  // "Unable to set WER parameter.  Continuing."

   hr = WerReportSetParameter(
      hReport,           // Handle to the report
      WER_P1,            // Identifier of the parameter to be set
      L"Resource Name",  // Unicode string that contains the name of the parameter
      resourceName       // The parameter value
   );
   WARN_HR_R( IDS_LOG_WER_FAILED_TO_SET_PARAMETER );  // "Unable to set WER parameter.  Continuing."
   /// @todo BUGFIX:  Replace the number with the name

   // Convert UINT resourceId to a wide string
   WCHAR szResourceId[ 8 ] = { 0 };
   StringCchPrintfW( szResourceId, 8, L"%u", resourceId );

   hr = WerReportSetParameter(
      hReport,           // Handle to the report
      WER_P2,            // Identifier of the parameter to be set
      L"Resource ID",    // Unicode string that contains the name of the parameter
      szResourceId       // The parameter value
   );
   WARN_HR_R( IDS_LOG_WER_FAILED_TO_SET_PARAMETER );  // "Unable to set WER parameter.  Continuing."

   hr = WerReportSetParameter(
      hReport,           // Handle to the report
      WER_P3,            // Identifier of the parameter to be set
      L"Message",        // Unicode string that contains the name of the parameter
      logMsg             // The parameter value
   );
   WARN_HR_R( IDS_LOG_WER_FAILED_TO_SET_PARAMETER );  // "Unable to set WER parameter.  Continuing."

   hr = WerReportSetParameter(
      hReport,           // Handle to the report
      WER_P4,            // Identifier of the parameter to be set
      L"Application Version",  // Unicode string that contains the name of the parameter
      FULL_VERSION_W     // The parameter value
   );
   WARN_HR_R( IDS_LOG_WER_FAILED_TO_SET_PARAMETER );  // "Unable to set WER parameter.  Continuing."
   /// @todo Pass the application version through parameters

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
