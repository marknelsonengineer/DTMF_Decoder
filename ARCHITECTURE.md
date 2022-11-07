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


## DTMF Decoder's Call Graph

![DTMF Decoder's Call Graph](./images/Call_Graph.svg)
