Architecture
============

The overall architecture of DTMF Decoder is fairly straightforward.

The main window is a 1-panel, hand-drawn, non-resizable window with a 
telephone keypad on it.

The program opens the default audio capture device (there's no user interface 
for selecting an audio device).  Then, it starts to listen to audio frames.

The most efficient way to listen to audio is to register a callback function 
that gets called when the OS has a batch of audio frames to process.  So,
we spin up an audio capture thread, wait for audio, process the frames and
then release the buffer.

We use a [Goertzel Algorithm](https://en.wikipedia.org/wiki/Goertzel_algorithm)
to determine how much energy is in each frequency bucket.  This implementation
processes the time-domain PCM data in 1 pass (for each frequency) -- and 
because DTMF needs to monitor 8 frequencies, it needs 8 passes/calculations.  

For efficiency (and for fun) I chose to spin up 8 Goertzel Work Threads, 
which wait (in parallel) for a batch of frames to come in.  Once they arrive,
it signals all 8 threads to run in parallel and when **all** of the Goertzel
work threads finish, it signals the Audio Capture thread to continue 
processing.

When a frequency surpasses a set threshold, the row or column frequency labels
are redrawn (in a highlighted color).  If both a DTMF row *and* column are 
"on", then the key "lights up" as well.  Super simple.

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
Language.  C is very "chatty" with memory, and this algorithm takes advantage
of:  
  - Out-of-order processing
  - All of the intermediate variables are held in registers

The x86-32 bit version of the program uses a traditional C-based Goertzel 
algorithm, for a reference design and comparison.

When you are running DTMF Decoder in a VM, it's still subject to the whims
of the hypervisor's scheduler.  Therefore, you may get frames with
DATA_DISCONTINUITY set.  However, when you run it on a bare-metal
Windows system, the performance is excellent and it processes all of the 
audio in realtime.

DTMF Decoder has a good, simple logging mechanism.  It logs everything to 
DebugView.  Logs to WARN, ERROR and FATAL will also popup a Dialog Box.  That's
it for a user interface.

So, it's a very simple program that took ~80 hours to write (4,800 minutes). 
There's ~3,000 lines of code, so I'm averaging about 96-seconds per line.  Not 
bad, per [The Mythical Man Month](https://en.wikipedia.org/wiki/The_Mythical_Man-Month)
but still, not great.

That said, it's been forever since I've written in Win32 and I'd write this
much faster if I had to do it again.  I've written a guide to the API calls I 
use [here](REFERENCES.md).  I read every one of these to write this program.

## Shutdown
I've never had to think quite so hard about how to (correctly) shutdown a
program as I've had for a [Win32](https://learn.microsoft.com/en-us/windows/win32/) 
program.  There are so many scenarios to consider!  For example, shutting down...
   - Before any windows have been created
   - During the program's initialization (before the message loop has started)
   - In a thread (that has its own synchronization and doesn't spin the
     message loop)
   - A failure while cleaning up during shutdown
   - ...and a normal shutdown
   
Yikes!

Here's what I've learned about processes' end-of-life:
   - [Error Handling](https://learn.microsoft.com/en-us/windows/win32/debug/error-handling) by Microsoft
     - Well-written applications include error-handling code that allows them 
       to recover gracefully from unexpected errors. When an error occurs, the 
       application may need to request user intervention, or it may be able to 
       recover on its own.
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

### Subsystems to Consider
- **Main Window Thread**
  - Init Failure Handling
    - In `wWinMain`, before starting the message loop... the `initSomething` functions
      should return `BOOL`s.  If they return `FALSE`, then run #gracefulShutdown and continue??.
      The message loop will start, but quickly terminate and shutdown the other
      subsystems.      
      
  - Running Failure Handling
    - Mostly, this happens in the other subsystems, however:
      - (Done) Problems in the message loop will log an error and call #gracefulShutdown
      - Problems in the message handler code will also log issues and call 
        #gracefulShutdown if necessary
    
  - Normal Shutdown
    - #GracefulShutdown will set #gbIsRunning to `false` and post a `WM_CLOSE` message.
    - WM_CLOSE will
      - Call #goertzel_end
      - Cause the Audio capture thread to loop (and terminate) by setting the `AudioSamplesReady` event
    Exits the message loop and runs to the end of wWinMain.
    - Call `DestroyWindow`
    - Tell the logger that the main window no longer exists
    - Exits the message loop and runs to the end of wWinMain.

- **GDI / Direct2D**
  - Init Failure Handling
  - Running Failure Handling
  - Normal Shutdown

- **Model**
  - Init Failure Handling
  - Running Failure Handling
  - Normal Shutdown

- **(Done) Logger**
  - Init Failure Handling
    - It's so simple that it's always successful (but we check it anyways)
  - Running Failure Handling
    - The logging functions can throw ASSERTs but they don't return error codes
  - Normal Shutdown
    - At the very end of the program

- **Audio Capture Thread**
  - Init Failure Handling
  - Running Failure Handling
  - Normal Shutdown

- **Goertzel DFT Threads**
  - Init Failure Handling
  - Running Failure Handling
  - Normal Shutdown


### Error Handling Policy
   - (Done) We will not use [TerminateProcess](https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-terminateprocess),
     [TerminateThread](https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-terminatethread)
     or [FatalAppExitA](https://learn.microsoft.com/en-us/windows/win32/api/errhandlingapi/nf-errhandlingapi-fatalappexita)
     they are too big of a gun for this application.
   - Instead of using [ExitProcess](https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-exitprocess),
     the preference would be for [wWinMain](https://learn.microsoft.com/en-us/windows/win32/learnwin32/winmain--the-application-entry-point)
     to exit.
     - (Done) A normal exit returns 0
     - An abnormal exit returns someting else... but it does not have to be 
       unique for each type of error
     - (Done) The model will hold `giApplicationReturnValue`, which will initially
       be set to 0 (Success).  Then, any function/error handler can set it
       which will then get passed out when the program terminates.
   - (Done) `gbIsRunning` is set to `true` at exactly one place:  The start of `wWinMain`
   - (Done) I've considered using [FlashWindow](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-flashwindow)
     for errors or warnings, but that's not really what it's for.  
     The MessageBox approach used by the logger is closer to the [Principle of Least Astonishment](https://en.wikipedia.org/wiki/Principle_of_least_astonishment)
   - **If the main window hasn't started**
     - Show a dialog box, then exit `wWinMain`
   - **During initialization, but the message loop hasn't started yet 
     (the `initSomething()` functions)**
     - Unwind, returning `FALSE` until you get to the top, then show a MessageBox
       and then exit `wWinMain`
   - **After the message loop has started**
     - Send `WM_CLOSE` to the main window
   - **A normal close operation**
     - On `WM_CLOSE` - The start of a normal close... start unwinding things
       - Set `isRunning` to `false`
       - Call `goertzel_end`
       - Trigger the audio capture thread loop
         - When `WaitForSingleObject()` returns, it will see that `isRunning`
           is `FALSE`, terminate the loop, then cleanup all things audio.
       - Call `audioStopDevice()` ??? Need to think about this ???
       - Call `destroyWindow()` - This is standard Win32 behavior
       - Call logCleanup() - TODO:  This definately needs to move
     - On `WM_DESTROY` 
       - Cleanup the view
       - Cleanup Goertzel
       - `PostQuitMessate( giApplicationReturnValue )` - This is standard Win32 behavior

### Macros & Functions Supporting Shutdown
- #gracefulShutdown - Initiates a shutdown by posting WM_CLOSE and setting
                      #gbIsRunning to `false`
- #gbIsRunning - A global `bool` that all of the worker threads watch (and spin)
- #CHECK_HR - Check a handle return value and return with a FAIL
- WARN_HR - Maybe we need to write this??
- #CHECK_BR - Check a `BOOL` return value
- #WARN_BR - Check a `BOOL` return value
- #CHECK_IR - Check an `int` return value

## Coding Conventions
The [C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines) 
discourage prefix notation (for example, Hungarian notation).  Internally, the
Windows team no longer uses it. But its use remains in samples and documentation.

I choose to continue to use it.  See [Windows Coding Convention](https://learn.microsoft.com/en-us/windows/win32/learnwin32/windows-coding-conventions).


## DTMF Decoder's Call Graph

![DTMF Decoder's Call Graph](./images/Call_Graph.svg)
