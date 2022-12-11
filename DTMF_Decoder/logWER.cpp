///////////////////////////////////////////////////////////////////////////////
//          University of Hawaii, College of Engineering
//          DTMF_Decoder - EE 469 - Fall 2022
//
//  A Windows Desktop C program that decodes DTMF tones
//
/// Extend log.cpp to use the Windows Error Reporting (WER) utility
///
/// This utility extends log.cpp.  It generates a Windows Error Report if the
/// application has a fatal error.
///
/// The data logWER collects is:
///   - The application's version
///   - For the first #LOG_LEVEL_ERROR or #LOG_LEVEL_FATAL message:
///     - The log level
///     - The error message
///     - The resource name
///     - The resource id
///   - The first 4K of log messages
///   - The last 4K of log messages (in a circular buffer)
///
/// The design is such that the main application doesn't need to know much
/// about logWER.
///
/// Applications need to be digitally signed before they can be recorded in
/// Microsoft's live WER database... where developers can get access to
/// analytics and the application's health.
///
/// Users should see the logger's `MessageBox`-based messages but not the WER
/// dialog box.
///
/// @see https://learn.microsoft.com/en-us/windows/win32/wer/windows-error-reporting
/// @see Windows Quality Online Services
/// @see https://en.wikipedia.org/wiki/Winqual
///
/// @todo Consider getting into making our own UI for error reporting via `WerReportSetUIOption` https://learn.microsoft.com/en-us/windows/win32/api/werapi/nf-werapi-werreportsetuioption
/// @todo Consider getting into custom metadata via `WerRegisterCustomMetadata` https://learn.microsoft.com/en-us/windows/win32/api/werapi/nf-werapi-werregistercustommetadata
///
/// If you are into WER, checkout an excellent paper on the subject:
/// [Debugging in the (Very) Large: Ten Years of Implementationand Experience](https://www.sigops.org/s/conferences/sosp/2009/papers/glerum-sosp09.pdf)
///
/// ## Generic Win32 API
/// | API                        | Link                                                                                                         |
/// |----------------------------| -------------------------------------------------------------------------------------------------------------|
/// | `LoadStringW`              | https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-loadstringw                           |
/// | `GetCurrentProcess`        | https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-getcurrentprocess |
/// | `GetProcessImageFileNameW` | https://learn.microsoft.com/en-us/windows/win32/api/psapi/nf-psapi-getprocessimagefilenamew                  |
///
/// ## Debugging& Instrumentation API
/// | API                      | Link                                                                                                                |
/// |--------------------------| --------------------------------------------------------------------------------------------------------------------|
/// | `WerRegisterMemoryBlock` | https://learn.microsoft.com/en-us/windows/win32/api/werapi/nf-werapi-werregistermemoryblock                         |
/// | `WerReportAddDump`       | https ://learn.microsoft.com/en-us/windows/win32/api/werapi/nf-werapi-werreportadddump                              |
/// | `WerReportCloseHandle`   | https ://learn.microsoft.com/en-us/windows/win32/api/werapi/nf-werapi-werreportclosehandle                          |
/// | `WerReportCreate`        | https ://learn.microsoft.com/en-us/windows/win32/api/werapi/nf-werapi-werreportcreate                               |
/// | `WerReportSetParameter`  | https ://learn.microsoft.com/en-us/windows/win32/api/werapi/nf-werapi-werreportsetparameter                         |
/// | `WerReportSubmit`        | https ://learn.microsoft.com/en-us/windows/win32/api/werapi/nf-werapi-werreportsubmit                               |
/// | `_ASSERTE`               | https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/assert-asserte-assert-expr-macros?view=msvc-170   |
/// | `wcslen`                 | https://en.cppreference.com/w/c/string/wide/wcslen                                                                  |
/// | `va_arg`                 | https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/va-arg-va-copy-va-end-va-start?view=msvc-170      |
///
/// ## CRT & Memory Management API
/// | API                | Link                                                                                         |
/// |--------------------| ---------------------------------------------------------------------------------------------|
/// | `CopyMemory`       | https://learn.microsoft.com/en-us/previous-versions/windows/desktop/legacy/aa366535(v=vs.85) |
/// | `StringCbCopyExW`  | https://learn.microsoft.com/en-us/windows/win32/api/strsafe/nf-strsafe-stringcbcopyexw       |
/// | `StringCbPrintfW`  | https://learn.microsoft.com/en-us/windows/win32/api/strsafe/nf-strsafe-stringcbprintfw       |
/// | `StringCchPrintfW` | https://learn.microsoft.com/en-us/windows/win32/api/strsafe/nf-strsafe-stringcchprintfw      |
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

#define REPORT_NAME_SIZE 64  ///< The maximum size of #swzReportName

#define MESSAGE_BUFFER_CAPACITY 4096   ///< Define the size of #msgBuf_t.msgBuf


static HREPORT shReport = NULL;                            ///< Handle to the windows error report
static WCHAR   swzFullExeFilename[ MAX_PATH ]    = { 0 };  ///< Full path of the executable file name
static WCHAR   swzReportName[ REPORT_NAME_SIZE ] = { 0 };  ///< Name of the report
static BOOL    sbLoggedWerFatalEvent = FALSE;              ///< WER captures the first #LOG_LEVEL_ERROR or #LOG_LEVEL_FATAL event.  This is `TRUE` if an event has been logged.


/// The structure of the two WER buffers
///
/// Integers are held as bytes as that's easier to read as a memory dump
///
/// @todo Create and monitor a guard for this buffer
typedef struct {
   size_t messageBufferCapacity;  ///< The size of the message buffer in bytes
   size_t msgStartOffset;         ///< The offset (in bytes!) to the end of the buffer (or the wraparound point)
   WCHAR msgBuf[ MESSAGE_BUFFER_CAPACITY ];  ///< The message buffer
} msgBuf_t;

/// A buffer with the first #MESSAGE_BUFFER_CAPACITY characters of the log.
/// Once data is written into this buffer, it never gets overwritten.  This
/// buffer preserves the startup logs of the application.
static msgBuf_t firstMsgs = { MESSAGE_BUFFER_CAPACITY << 1, 0, { 0 } };

/// `FALSE` while #firstMsgs is filling.  Once it's full, set to `TRUE` and
/// #lastMsgs will start filling.
static BOOL bIsFirstBufferFull = FALSE;

/// A circular buffer with the last #MESSAGE_BUFFER_CAPACITY characters of the
/// log.  After the initial log is written in #firstMsgs, log messages are
/// written (circularly) in this buffer.  This buffer preserves the events
/// leading up to the failure.
static msgBuf_t lastMsgs = { MESSAGE_BUFFER_CAPACITY << 1, 0, { 0 } };


/// Initialize Windows Error Reporting
///
/// As an extension to log.cpp, logWER.cpp can see all of the variables set in
/// #logInit.
///
/// @return `TRUE` if successful.  `FALSE` if there was a problem.
RETURN_BOOL logWerInit() {
   DWORD   dwr;  // DWORD result
   HRESULT hr;   // HRESULT result

   HANDLE hProcess = GetCurrentProcess();
   // No error checking available

   dwr = GetProcessImageFileNameW( hProcess, swzFullExeFilename, MAX_PATH );
   if ( dwr <= 0 ) {
      RETURN_FATAL( IDS_LOG_WER_FAILED_TO_GET_EXE_PATH );  // "Failed to get the executable's full path name"
   }
   LOG_INFO_R( IDS_LOG_WER_FULL_EXE_FILENAME, swzFullExeFilename );  // "GetProcessImageFileNameW Full Exe Filename=%s"


   WER_REPORT_INFORMATION myReport = { 0 };

   myReport.dwSize = sizeof( myReport );
   myReport.hProcess = NULL;  // Handle to the report's process, if `NULL` it's the calling process
   CopyMemory( myReport.wzConsentKey,        swAppName,          sizeof( myReport.wzConsentKey        ) );
   CopyMemory( myReport.wzFriendlyEventName, swAppTitle,         sizeof( myReport.wzFriendlyEventName ) );
   CopyMemory( myReport.wzApplicationName,   swAppTitle,         sizeof( myReport.wzApplicationName   ) );
   CopyMemory( myReport.wzApplicationPath,   swzFullExeFilename, sizeof( myReport.wzApplicationPath   ) );

   // Compose myReport.wzDescription
   wBuffer_t format = { L"", BUFFER_GUARD };
   logGetStringFromResources( IDS_LOG_WER_DESCRIPTION, &format );  // "%s has a problem and needs to shutdown.  A report will be generated and sent to the developer."
   StringCbPrintfW( myReport.wzDescription, sizeof( myReport.wzDescription ), format.sBuf, swAppTitle );

   myReport.hwndParent = *sphMainWindow;

   // Compose swzReportName
   format = { L"", BUFFER_GUARD };
   logGetStringFromResources( IDS_LOG_WER_REPORT_NAME, &format );  // "%s Error Report"
   StringCchPrintfW( swzReportName, REPORT_NAME_SIZE, format.sBuf, swAppTitle );

   hr = WerReportCreate(
      swzReportName,       // A Unicode string with the name of this event
      WerReportCritical,   // The type of report
      &myReport,           // A pointer to a WER_REPORT_INFORMATION structure
      &shReport );         // A handle to the report. If the function fails, this handle is NULL.

   if ( hr != S_OK || shReport == NULL ) {
      LOG_WARN_R( IDS_LOG_WER_FAILED_CREATE_REPORT );  // "Failed to create a Windows Error Report (should it be needed later).  Continuing."
   }

   sbLoggedWerFatalEvent = FALSE;

   LOG_INFO_R( IDS_LOG_WER_INIT_SUCCESS );  // "Initialized Windows Error Reporting."

   return TRUE;
}


/// Journal a log message into one of the two WER log queues
///
/// @param logLevel      The level of this logging event
/// @param resourceName  The name of the resource (if any)
/// @param resourceId    The ID of the resource (if any)
/// @param logMsg        The fully expanded message to log
///
/// @return `TRUE` if successful.  `FALSE` if there was a problem.
RETURN_BOOL logWerEvent(
   _In_       const logLevels_t logLevel,
   _In_opt_z_ const PCWSTR      resourceName,
   _In_       const UINT        resourceId,
   _In_z_     const PCWSTR      logMsg
   ) {
/// @todo Figure this out so we can uncomment it (clang-tidy throws errors when we Analyze it)
//   if ( resourceName != NULL ) {
//      _ASSERTE( resourceName[ 0 ] != L'\0' );
//      _ASSERTE( resourceId != 0 );
//   }

   if ( resourceId != 0 ) {
      _ASSERTE( resourceName != NULL );
      _ASSERTE( resourceName[ 0 ] != L'\0' );
   }
   _ASSERTE( logMsg != NULL );
   _ASSERTE( logMsg[ 0 ] != L'\0' );

   HRESULT hr;  // HRESULT result

   _ASSERTE( firstMsgs.messageBufferCapacity >= firstMsgs.msgStartOffset );
   _ASSERTE( lastMsgs.messageBufferCapacity >= lastMsgs.msgStartOffset );

   size_t msgLength = wcslen( logMsg ) << 1;  // wcslen returns the number of chars, << 1 converts to bytes

   size_t firstBytesRemaining = firstMsgs.messageBufferCapacity - firstMsgs.msgStartOffset;

   /// If the current message would overflow #firstMsgs, then it's full and
   /// it's time to shift to the #lastMsgs buffer.
   if ( !bIsFirstBufferFull && msgLength >= firstBytesRemaining ) {
      bIsFirstBufferFull = TRUE;
   }

   STRSAFE_LPWSTR endPtr;

   if ( !bIsFirstBufferFull ) {  // If #firstMsgs is not full...
      _ASSERTE( firstBytesRemaining > msgLength );

      hr = StringCbCopyExW(
         firstMsgs.msgBuf + ( firstMsgs.msgStartOffset >> 1 ),  // Destination buffer
         firstBytesRemaining,                   // Size of destination buffer in bytes
         logMsg,                                // Null terminated wide source string
         &endPtr,                               // Populate a pointer to the end of the destination buffer
         NULL,                                  // Number of unused bytes in destination
         0                                      // Flags
      );
      if ( !( hr == S_OK || hr == STRSAFE_E_INSUFFICIENT_BUFFER ) ) {
         FATAL_IN_LOG( L"Unknown error in StringCbCopyExW.  Exiting immediately." );
      }

      // Pointer arithmetic is always in terms of the derefrenced object
      // (`WCHAR`s in this case), so convert them to bytes.
      firstMsgs.msgStartOffset = ( endPtr - firstMsgs.msgBuf ) << 1;
   } else {
      size_t lastBytesRemaining = lastMsgs.messageBufferCapacity - lastMsgs.msgStartOffset;

      // If this message exceeds the remaining size of the buffer, then start over
      if ( msgLength >= lastBytesRemaining ) {
         lastMsgs.msgStartOffset = 0;
         lastBytesRemaining = lastMsgs.messageBufferCapacity;
      }

      hr = StringCbCopyExW(
         lastMsgs.msgBuf + ( lastMsgs.msgStartOffset >> 1 ),  // Destination buffer
         lastBytesRemaining,                    // Size of destination buffer in bytes
         logMsg,                                // Null terminated wide source string
         &endPtr,                               // Populate a pointer to the end of the destination buffer
         NULL,                                  // Number of unused bytes in destination
         0                                      // Flags
      );
      if ( !( hr == S_OK || hr == STRSAFE_E_INSUFFICIENT_BUFFER ) ) {
         FATAL_IN_LOG( L"Unknown error in StringCbCopyExW.  Exiting immediately." );
      }

      // Pointer arithmetic is always in terms of the derefrenced object
      // (`WCHAR`s in this case), so convert them to bytes.
      lastMsgs.msgStartOffset = ( endPtr - lastMsgs.msgBuf ) << 1;
   }

   /// Don't set any WER parameters if the log level is less than #LOG_LEVEL_ERROR
   if ( logLevel < LOG_LEVEL_ERROR ) {
      return TRUE;
   }

   /// Only one (the first #LOG_LEVEL_WARN or #LOG_LEVEL_FATAL log message) can
   /// fully parameterized/characterized in a report to WER
   if ( sbLoggedWerFatalEvent ) {
      return TRUE;
   }

   hr = WerReportSetParameter(
      shReport,        // Handle to the report
      WER_P0,          // Identifier of the parameter to be set
      L"Application Version",  // Unicode string that contains the name of the parameter
      FULL_VERSION_W   // The parameter value
                       /// @todo Pass the application's version through a parameter
   );
   WARN_HR_R( IDS_LOG_WER_FAILED_TO_SET_PARAMETER, WER_P0 );  // "Failed to set WER parameter:  %d   Continuing."

   hr = WerReportSetParameter(
      shReport,      // Handle to the report
      WER_P1,        // Identifier of the parameter to be set
      L"Log Level",  // Unicode string that contains the name of the parameter
      ( logLevel == LOG_LEVEL_ERROR ) ? L"ERROR" : L"FATAL"   // The parameter value
   );
   WARN_HR_R( IDS_LOG_WER_FAILED_TO_SET_PARAMETER, WER_P1 );  // "Failed to set WER parameter:  %d   Continuing."

   hr = WerReportSetParameter(
      shReport,        // Handle to the report
      WER_P2,          // Identifier of the parameter to be set
      L"Message",      // Unicode string that contains the name of the parameter
      logMsg           // The parameter value
   );
   WARN_HR_R( IDS_LOG_WER_FAILED_TO_SET_PARAMETER, WER_P2 );  // "Failed to set WER parameter:  %d   Continuing."

   if ( resourceName != NULL ) {
      hr = WerReportSetParameter(
         shReport,          // Handle to the report
         WER_P3,            // Identifier of the parameter to be set
         L"Resource Name",  // Unicode string that contains the name of the parameter
         resourceName       // The parameter value
      );
      WARN_HR_R( IDS_LOG_WER_FAILED_TO_SET_PARAMETER, WER_P3 );  // "Failed to set WER parameter:  %d   Continuing."
   }

   if ( resourceId != 0 ) {
   // Convert UINT resourceId to a wide string
      WCHAR szResourceId[ 8 ] = { 0 };
      StringCchPrintfW( szResourceId, 8, L"%u", resourceId );

      hr = WerReportSetParameter(
         shReport,        // Handle to the report
         WER_P4,          // Identifier of the parameter to be set
         L"Resource ID",  // Unicode string that contains the name of the parameter
         szResourceId     // The parameter value
      );
      WARN_HR_R( IDS_LOG_WER_FAILED_TO_SET_PARAMETER, WER_P4 );  // "Failed to set WER parameter:  %d   Continuing."
   }

   LOG_INFO_R( IDS_LOG_WER_FATAL_ERROR_LOGGED );  // "WER fatal error logged"

   sbLoggedWerFatalEvent = TRUE;

   return TRUE;
}


/// Submit a Windows Error Report to Microsoft
///
/// @return `TRUE` if successful.  `FALSE` if there was a problem.
RETURN_BOOL logWerSubmit() {
   HRESULT hr;  // HRESULT result

   hr = WerRegisterMemoryBlock(
      &firstMsgs, sizeof( firstMsgs )
   );
   CHECK_HR_R( IDS_LOG_WER_FAILED_TO_REGISTER_MEMORY, L"First" );  // "Failed to register %s memory block with Windows Error Report"

   hr = WerRegisterMemoryBlock(
      &lastMsgs, sizeof( lastMsgs )
   );
   CHECK_HR_R( IDS_LOG_WER_FAILED_TO_REGISTER_MEMORY, L"Last" );  // "Failed to register %s memory block with Windows Error Report"


   hr = WerReportAddDump(
      shReport,
      GetCurrentProcess(),
      GetCurrentThread(),
      WerDumpTypeHeapDump,  // Dump type
      NULL,                 // Exception parameters
      NULL,                 // Custom options
      0 );                  // Flags
   CHECK_HR_R( IDS_LOG_WER_FAILED_TO_ADD_DUMP );  // "Failed to add dump to Windows Error Report"


   WER_SUBMIT_RESULT submissionResult;

   hr = WerReportSubmit(
      shReport,            // The report handle
      WerConsentApproved,  // The consent status enum
      WER_SUBMIT_NO_CLOSE_UI | WER_SUBMIT_QUEUE | WER_SUBMIT_REPORT_MACHINE_ID | WER_SUBMIT_ADD_REGISTERED_DATA,
      &submissionResult
   );
   CHECK_HR_R( IDS_LOG_WER_FAILED_TO_SUBMIT_REPORT );  // "Failed to submit Windows Error Report"

   LOG_TRACE_R( IDS_LOG_WER_SUCCESSFULLY_SUBMITTED );  // "Windows Error Report submitted successfully"

   return TRUE;
}


/// Cleanup Windows Error Reporting resources
///
/// @return `TRUE` if successful.  `FALSE` if there was a problem.
RETURN_BOOL logWerCleanup() {
   HRESULT hr;

   if ( shReport != NULL ) {
      hr = WerReportCloseHandle( shReport );
      CHECK_HR_R( IDS_LOG_WER_FAILED_TO_CLOSE_HANDLE );  // "Failed to close WER handle"
   }

   sbLoggedWerFatalEvent = FALSE;

   shReport = NULL;
   return TRUE;
}
