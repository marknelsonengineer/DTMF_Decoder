Release Checklist
=================

This checklist is for a Visual Studio Win32 C Windows program

- For each source file:
    - Cleanup `#include` files
    - Ensure all parameters are validated with `_ASSERTE`
    - Ensure all parameters have [SAL](https://learn.microsoft.com/en-us/cpp/code-quality/using-sal-annotations-to-reduce-c-cpp-code-defects?view=msvc-170) annotations
    - Look for `const`-able parameters
    - Is everything documented?

- Doxygen
    - No Doxygen warnings
    - Proofread the Doxygen-generated content

- Last looks
    - Read through all of the `@todo`s
    - Do a `Rebuild Project` without any warnings
    - Regenerate the statistics:

          wsl
          $ cd ~/src/VisualStudio/DTMF_Decoder
          $ ./bin/stats.sh | tee STATISTICS.md

    - Run [DOT Online](https://dreampuf.github.io/GraphvizOnline) to regenerate 
      the call diagram
