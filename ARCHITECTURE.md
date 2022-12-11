Architecture
============

The overall architecture of DTMF Decoder is very straightforward.

The main interface is a 1-panel, hand-drawn, non-resizable window with a
telephone keypad on it.

The program opens the default audio capture device (there's no user interface
for selecting an audio device).  It then starts to listen to audio frames.

The most efficient way to listen to audio is to register a callback function
that gets called when the OS has a batch of audio frames to process.  So,
we spin up an audio capture thread, wait for audio, process the frames and
then release the buffer.

We use a [Goertzel Algorithm](https://en.wikipedia.org/wiki/Goertzel_algorithm)
to determine how much energy is in each DTMF frequency bucket.  This implementation
processes the time-domain PCM data in 1 pass (for each frequency) -- and
because DTMF needs to monitor 8 frequencies, it needs 8 passes/calculations.

For efficiency (and for fun) I chose to spin up 8 Goertzel Work Threads,
which wait (in parallel) for a batch of frames to come in.  When they arrive,
it signals all 8 threads to run in parallel and when **all** of the Goertzel
work threads finish, it signals the Audio Capture thread to continue
processing.

When the energy at a given frequency surpasses a set threshold, the row or
column frequency labels are redrawn (in a highlighted color).  If both a
DTMF row *and* column are "on", then the key "lights up" as well.  Super simple.

Per [Raymond Chen](https://devblogs.microsoft.com/oldnewthing/author/oldnewthing)'s
book [The Old New Thing](https://www.amazon.com/Old-New-Thing-Development-Throughout/dp/0321440307/)
I should trust the Window's Update mechanism and repaint the display on the
main thread in WM_PAINT.  To that end, when each Goertzel thread finishes its
calculations, it checks to see if the state has changed.  If so, it invalidates
a rectangle (a full row or column) on the display.  Later on, the main application
message loop will get a WM_PAINT message and the screen will repaint.

Each element that gets drawn on the screen checks an "update rectangle".  If
the element is in the update rectangle, it gets drawn.  If not, then it must
not need to be updated.  This is the Win32 way of drawing.

For performance, I hand-coded an x86-64 Goertzel algorithm in Assembly
Language.  C is very "chatty" with memory.   An Assembly Language based
algorithm takes advantage of:
  - Out-of-order processing
  - All of the intermediate variables are held in registers

The x86-32 bit version of the program uses a traditional C-based Goertzel
algorithm for a reference design and comparison.

When you are running DTMF Decoder in a VM, it's still subject to the whims
of the hypervisor's scheduler.  Therefore, you may get frames with
`DATA_DISCONTINUITY` set.  However, when you run it on a bare-metal
Windows system, the performance is excellent and it processes all of the
audio in realtime.

DTMF Decoder has a good, simple logging mechanism.  It logs everything to
DebugView.  Logs to WARN, ERROR and FATAL will also show a Dialog Box.
DTMF Decoder has an About dialog box and that's it for a user interface.

So, it's a very simple program that took ~80 hours to write (4,800 minutes).
There's ~3,000 lines of code, so I'm averaging about 96-seconds per line.  Not
bad, per [The Mythical Man Month](https://en.wikipedia.org/wiki/The_Mythical_Man-Month)
but still, not great.

That said, it's been forever since I've written in Win32 and I'd write this
much faster if I had to do it again.  I've written a guide to the API calls I
use [here](REFERENCES.md).  I read every one of them to write this program.


## Shutdown
I've never had to work quite so hard to (correctly) shutdown a program as I've
had to for a [Win32](https://learn.microsoft.com/en-us/windows/win32/)
program.  There are so many scenarios to consider!  For example, shutting down...
   - Before any windows have been created
   - During the program's initialization (before the message loop has started)
   - In a thread (that has its own synchronization and doesn't spin the
     message loop)
   - A failure while cleaning up during shutdown (after the main window
     has been destroyed)
   - ...and a normal shutdown

In keeping with the goals of this project, my intention is to:
   - Test every API call for errors (It's so easy to ignore things in Windows)
   - Keep the user informed any error conditions
   - Correctly handle every error condition

This is ridiculously hard to do in Win32.  Let's start with what I've learned
about processes' end-of-life:
   - [Error Handling](https://learn.microsoft.com/en-us/windows/win32/debug/error-handling) by Microsoft
     - Well-written applications include error-handling code that allows them
       to recover gracefully from unexpected errors.  When an error occurs, the
       application may need to request user intervention, or it may be able to
       recover on its own.
   - [Modern C++ Best Practices for Exceptions and Error Handling](https://learn.microsoft.com/en-us/cpp/cpp/errors-and-exception-handling-modern-cpp?view=msvc-170)
     - I do not plan to re-architect a Win32 program to use exceptions.
     - The native Win32 API doesn't seem to use C++ exceptions either.
   - Work threads shouldn't display `MessageBox`es, so we need to queue a message,
     initiate a shutdown and then print a `MessageBox` as it's winding down.
   - [The WM_QUIT_Message](https://devblogs.microsoft.com/oldnewthing/20050222-00/?p=36393)
   - [The difference between WM_QUIT, WM_CLOSE, and WM_DESTROY](https://stackoverflow.com/questions/3155782/what-is-the-difference-between-wm-quit-wm-close-and-wm-destroy-in-a-windows-pr)
   - [Terminating a Process](https://learn.microsoft.com/en-us/windows/win32/procthread/terminating-a-process)
   - [Terminating a Thread](https://learn.microsoft.com/en-us/windows/win32/procthread/terminating-a-thread)
   - I ran Spy++ on DTMF Decoder and captured the messages when I clicked the
     X in the upper-right corner.

         <000075> P WM_NCLBUTTONDOWN nHittest:HTCLOSE xPos:443 yPos:7
         <000076> P WM_MOUSEMOVE fwKeys:MK_LBUTTON xPos:439 yPos:-44
         <000077> P WM_LBUTTONUP fwKeys:0000 xPos:439 yPos:-44
         <000078> S WM_CAPTURECHANGED hwndNewCapture:00000000
         <000079> R WM_CAPTURECHANGED
         <000080> S WM_SYSCOMMAND uCmdType:SC_CLOSE xPos:443 yPos:7
         <000081> S WM_CLOSE
         <000082> S message:0x0090 [Unknown] wParam:00000000 lParam:00000000
         <000083> R message:0x0090 [Unknown] lResult:00000000
         <000084> S WM_WINDOWPOSCHANGING lpwp:0000001D5472DE00
         <000085> R WM_WINDOWPOSCHANGING
         <000086> S WM_WINDOWPOSCHANGED lpwp:0000001D5472DE00
         <000087> R WM_WINDOWPOSCHANGED
         <000088> S WM_NCACTIVATE fActive:False
         <000089> S message:0x0093 [Unknown] wParam:00000000 lParam:0000001D5472CB40
         <000090> R message:0x0093 [Unknown] lResult:00000001
         <000091> S message:0x0093 [Unknown] wParam:00000000 lParam:0000001D5472D460
         <000092> R message:0x0093 [Unknown] lResult:00000001
         <000093> S message:0x0091 [Unknown] wParam:00000000 lParam:0000001D5472D460
         <000094> R message:0x0091 [Unknown] lResult:00000000
         <000095> S message:0x0092 [Unknown] wParam:00000000 lParam:0000001D5472D3E0
         <000096> R message:0x0092 [Unknown] lResult:00000000
         <000097> S message:0x0092 [Unknown] wParam:00000000 lParam:0000001D5472D3E0
         <000098> R message:0x0092 [Unknown] lResult:00000000
         <000099> R WM_NCACTIVATE fDeactivateOK:True
         <000100> S WM_ACTIVATE fActive:WA_INACTIVE fMinimized:False hwndPrevious:(null)
         <000101> R WM_ACTIVATE
         <000102> S WM_ACTIVATEAPP fActive:False dwThreadID:00000DA8
         <000103> R WM_ACTIVATEAPP
         <000104> S WM_KILLFOCUS hwndGetFocus:(null)
         <000105> R WM_KILLFOCUS
         <000106> S WM_IME_SETCONTEXT fSet:0 iShow:C000000F
         <000107> S WM_IME_NOTIFY dwCommand:IMN_CLOSESTATUSWINDOW dwCommand:00000001 dwData:00000000
         <000108> R WM_IME_NOTIFY
         <000109> R WM_IME_SETCONTEXT
         <000110> S WM_DESTROY
         <000111> R WM_DESTROY
         <000112> S WM_NCDESTROY
         <000113> R WM_NCDESTROY
         <000114> R WM_CLOSE
         <000115> R WM_SYSCOMMAND

   - I can see `WM_CLOSE` and `WM_DESTROY`
   - `WM_QUIT` never makes it to the message handler... which makes sense, as
     soon as `GetMessage` gets it, it exits out of the `while()` loop.
   - [The convention for process exit codes](https://peteronprogramming.wordpress.com/2018/05/29/list-of-peculiar-exit-codes-on-windows/)
     in Windows is:
     - `0` means success
     - Anything else means failure
     - [Standard Windows application return codes](https://stackoverflow.com/questions/1538884/what-standard-application-return-exit-codes-should-an-application-support)
   - Win32 functions that return `BOOL` will return `0` on failure and non-`0` on
     success...  A program's exit code does the opposite -- they return `0` on
     success and non-`0` on failure.
   - Be mindful that Win32's `BOOL` datatype is an `INT`, not a `bool`.


### Error Handling Policy
   - All messages should be held as a multi-lingual capable string resource
     - There are a handful of messages (mostly around initialing the string
       resource API) that will be English-only.
       
   - On a fatal error, the message should end with "Exiting."  On a warning,
     the message should end with "Continuing."

   - #gbIsRunning is set to `true` at exactly one place:  The start of #wWinMain

   - **If the main window hasn't started**
     - Show a dialog box, then exit #wWinMain returnng #EXIT_FAILURE

   - **Error handling inside the `initSomething` functions**
     - The `initSomething` functions are called from #wWinMain before the
       message loop starts...

     - Each init function called by #wWinMain has a custom error handler.

      - The `initSomething` functions should return `BOOL`s.
        - If everything initializes OK, bubble up `TRUE`s
        - If there are problems...
          - Log the issue
          - Use `MessageBox` to display an error message
          - Set #giApplicationReturnValue to #EXIT_FAILURE
          - Unwind any initialization that's aready been done
            - Each init function has a unique set of cleanups depending on how
              far the initialization has progressed.
          - Bubble `FALSE` back up to #wWinMain

   - **After the message loop has started**
     - Set #giApplicationReturnValue to #EXIT_FAILURE
     - Queue a message to be displayed later
     - Call #gracefulShutdown

   - I've considered using [FlashWindow](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-flashwindow)
     for errors or warnings, but that's not really what it's for, so I'm
     not using it.  The `MessageBox` approach used in log.cpp is closer to the
     [Principle of Least Astonishment](https://en.wikipedia.org/wiki/Principle_of_least_astonishment)

   - The `cleanupSomething` methods should always work and not `ASSERT`, even
     if their corresponding `initSomething` methods haven't run yet.  This
     allows us to call all (actually, most) of the `cleanupSomething` methods
     during initialization failures.

   - Worker threads should not create windows (like `MessageBox`).  This bears
     repeating.  Worker threads should not create windows.  They will appear
     to hang, get out of sync and the controlling process will loose control
     of them.

     So, how can they communicate problems and safely shutdown the app?  The
     logger can queue messages and then play them back from the main thread.

   - **Ending the application (normally)**
     - On `WM_CLOSE` - The start of a normal close... start unwinding things
       - Set #gbIsRunning to `false`
       - Call `destroyWindow` - This is standard Win32 behavior

     - On `WM_DESTROY`
       - Cleanup the view by calling #mvcViewCleanup
       - Call `PostQuitMessate( giApplicationReturnValue )` - This is standard
         Win32 behavior

     - WM_QUIT will exit the message loop

     - As #wWinMain runs to the end... it will:
       - Stop the worker threads
       - Log the first WARN, ERROR or FATAL message (if any)
       - Cleanup resources in the reverse order they were created
       - Return with #giApplicationReturnValue

     - #wWinMain ends with `return giApplicationReturnValue;`
       - Instead of using [ExitProcess](https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-exitprocess),
         we use the return value of [wWinMain](https://learn.microsoft.com/en-us/windows/win32/learnwin32/winmain--the-application-entry-point)
         to exit.  I like the idea of letting the CRT (Consone runtime) library
         ending the program.
       - The model holds #giApplicationReturnValue, which is initially set to
         #EXIT_SUCCESS (`0`).  Any function/error handler can set it
         which gets passed out when the program terminates.
         - A normal exit returns #EXIT_SUCCESS (`0`)
         - An abnormal exit returns #EXIT_FAILURE
           - We don't need unique error codes for every type of error as we have
             very expressive logging

           - We will not use [TerminateProcess](https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-terminateprocess),
             [TerminateThread](https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-terminatethread)
             or [FatalAppExitA](https://learn.microsoft.com/en-us/windows/win32/api/errhandlingapi/nf-errhandlingapi-fatalappexita)
             they are too big of a gun for this application.

Normal and abnormal shutdowns can be differentiated by:

|                               | Normal Shutdown         | Abnormal Shutdown |
|-------------------------------|-------------------------|-------------------|
| Set #giApplicationReturnValue | #EXIT_SUCCESS (Default) | #EXIT_FAILURE     |
| Set #gbIsRunning to `false`   | Yes                     | Yes               |
| Post `WM_CLOSE`               | Yes                     | Yes               |
| Call #LOG_FATAL( message )    | No                      | Yes               |
| Functions return              | `TRUE`                  | `FALSE`           |


### Subsystems Shutdown Notes

- **GDI / Direct2D**
  - Init Error Handler
    - Set #giApplicationReturnValue to #EXIT_FAILURE
    - Call #LOG_FATAL
    - Call #gracefulShutdown
    - Return `FALSE` that bubbles up to the main thread
    - Because #gracefulShutdown was called, the main loop's
      messages won't display

  - Normal Shutdown
    - Cleaned up in `WM_DESTROY` immediately after the window is destroyed
    - The cleanup method is very simple and never fails

- **Model**
  - Init Error Handler
    - Doesn't do anything.  Always returns `TRUE` (for now).
  - Normal Shutdown
    - Go through each item in the model and zeros it out (or keep/ignore it)
    - Calls #pcmReleaseQueue

- **Logger**
  - Init Error Handler
    - It's so simple that it's always successful (but we check it anyways)
  - Running Error Handler
    - When Log functions throw exceptions, they will:

          OutputDebugStringA(   "VIOLATED STACK GUARD in Logger.  Exiting immediately." );
          _ASSERT_EXPR( FALSE, L"VIOLATED STACK GUARD in Logger.  Exiting immediately." );

    ... (because they terminate immediately, they don't return error codes)

- **Windows Error Reporting**
  - If the application ever generates a #LOG_LEVEL_ERROR or #LOG_LEVEL_FATAL,
    then the very first error message gets logged to WER and a report is
    submitted when the program exits.  If the application never generates
    these two errors, then it does not submit a report.

- **Audio Capture Thread**
  - None

- **Goertzel DFT Threads**
  - Init Error Handler
    - #goertzel_Init is called in #audioInit... after some important audio
      data structures are created but before the audio capture thread starts
  - Normal Shutdown
    - Call #goertzel_Stop to stop the threads.  #goertzel_Stop does not
      return until all of the threads have stopped
    - #goertzel_Stop is called _after_ ending the audio capture
      thread in #audioStop
    - Call #goertzel_Cleanup to close out the resources it created


### Macros & Functions Supporting Shutdown
- #gbIsRunning - A global `bool` that all of the worker threads watch

- #gracefulShutdown - Initiates a shutdown (can be normal shutdown or a shutdown
  due to an error).
  - Set #gbIsRunning to `false`
  - Post `WM_CLOSE`
  - #gracefulShutdown will not shutdown the program **before** the message
    loop starts; there's no queue to collect the `WM_CLOSE` yet
  - This is both a normal and failure-mode shutdown, so it doesn't set
    #giApplicationReturnValue

- Helper macros for checking the result of function calls (both to Win32 and
  internal calls).  All of these macros use wide resource strings and
  `printf`-style varargs.

  - These macros rely on pre-declared return variable declarations
    `BOOL br` and `HRESULT hr`_and_ they depend on the function being tested
    to put their return values into these variables.

  - They streamline a lot of ugly error checking code into:

        HRESULT hr;  // HRESULT result
        ...
        hr = D2D1CreateFactory( D2D1_FACTORY_TYPE_MULTI_THREADED, &spD2DFactory );
        CHECK_HR_R( IDS_VIEW_FAILED_TO_CREATE_DIRECT2D_FACTORY );  // "Failed to create Direct2D Factory"

#### Table of Macro Functions

|                              | Check a BOOL result   | Check an HRESULT      | Action taken          |
|------------------------------|-----------------------|-----------------------|-----------------------|
| Test for success or failure  | `if ( !br )`          | `if ( FAILED( hr ) )` |                       |
| Display a warning message    | #WARN_BR_R            | #WARN_HR_R            | Call #LOG_WARN_R      |
| Immediately display messages | #CHECK_BR_R           | #CHECK_HR_R           | Call #RETURN_FATAL_EX |
| Queue messages for later     | #CHECK_BR_Q           | #CHECK_HR_Q           | Call #QUEUE_FATAL_EX  |

  - #QUEUE_FATAL (just resource id) via #QUEUE_FATAL_EX (with resource name
    and id)
    - Sets #giApplicationReturnValue to #EXIT_FAILURE
    - Calls #LOG_FATAL_QX (the X log functions pass the resource name as a
      string)
    - Calls #gracefulShutdown

  - #PROCESS_FATAL (just resource id) via #PROCESS_FATAL_EX (with resource name
    and id)
    - Sets #giApplicationReturnValue to #EXIT_FAILURE
    - Calls #LOG_FATAL_RX (the X log functions pass the resource name as a string)
    - Calls #gracefulShutdown

  - #RETURN_FATAL (just resource id) and #RETURN_FATAL_EX (with resource name
    and id) are very simple... they call #PROCESS_FATAL_EX and then return
    `FALSE`


## Coding Conventions
The [C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines)
discourage prefix notation (for example, Hungarian notation).  Internally, the
Windows team no longer uses it, but its use remains in samples and
documentation.

I choose to continue to use it.
See [Windows Coding Convention](https://learn.microsoft.com/en-us/windows/win32/learnwin32/windows-coding-conventions).

By convention, most `extern` functions will be named `subjectAction`.  This
tends to group them together (by subject).  If the last letter of subject
is an `l`, then it's OK to name a function `subject_Action` as `l` is hard to
distinguish from `I`.


## DTMF Decoder's Call Graph

![DTMF Decoder's Call Graph](./images/Call_Graph.svg)
