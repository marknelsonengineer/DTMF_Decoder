# Architecture

The overall architecture of DTMF Decoder is fairly straightforward.

It's a 1-panel, hand-drawn, non-resizable window with a telephone keypad on it.

The program opens the default audio capture device (there's no user interface 
for selecting an audio device).  Then, it starts to listen to audio frames.

First thread

The DFT

The 8 threads

For permforance, I hand-coded an assembly 

### DTMF Decoder's Call Graph

![DTMF Decoder's Call Graph](./images/Call_Graph.svg)
