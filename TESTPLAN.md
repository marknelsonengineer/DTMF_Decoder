Testplan
========

This testplan can be run from any Configuration:  Release, Analyze or Debug... 
however it probably doesn't make sense to run it from Profile

## Basic tests
- Runs on a 64-bit OS
- Starts and draws the display
- Press `ESC` and the program exits
- Help > About works and is correct
  - The version number is correct
  - The build date is accurate
- Program processes / displays each of the digits:  
  `1 5 9 D * 8 6 A 2 6 C 4 8 # 0 7 4 2 3 B C # 1 1 1 1`
- No flicker as the keys/frequencies are updated
- When no tones are present, nothing displays / the output is steady even with
  some background noise
- Program detects individual tones
- Observe the program and each of the threads in Process Monitor and ensure none
  of the threads is over heating
- Run DebugView and observe all of the output


## Documentation
- Check the project's [GitHub page](https://github.com/marknelsonengineer/DTMF_Decoder) and make sure the home page looks good
- Check the project's [open issues](https://github.com/marknelsonengineer/DTMF_Decoder/issues) on GitHub and make sure they are accurate
- Check the [Doxygen content on UH Unix](https://www2.hawaii.edu/~marknels/DTMF_Decoder/) and make sure it looks good:
  - Check each of the .md files
  - Check a file, down to the source code
    - Ensure the comments in the source code are <span style="color:green">green</span>
    