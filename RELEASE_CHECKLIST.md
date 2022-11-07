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
    - Update the version number in:
      - Doxygen
      - Release > Linker > General > Version
    - Run Code Analysis without any warnings
    - Run [DOT Online](https://dreampuf.github.io/GraphvizOnline) to regenerate 
      the call diagram
      - Reconfigure Doxygen so it does NOT clean DOT files
      - Navigate to `./Doxygen/html` and get `DTMF__Decoder_8cpp_blah_blah_cgraph.dot` (it's the first of many)
      - Put the .DOT text into the copy buffer
      - Navigate to [https://dreampuf.github.io/GraphvizOnline](https://dreampuf.github.io/GraphvizOnline)
        and paste the .DOT file
      - Make the necessary changes to the .DOT file (see a Diff for examples)
      - Render and update `Call_Graph.svg`
      - Reconfigure Doxygen so it cleans DOT files (this significantly reduces
        the number of files we host at UH Unix)
    - Push the project into Github and Tag the release
    - Regenerate the statistics:

          wsl
          $ cd ~/src/VisualStudio/DTMF_Decoder
          $ ./bin/stats.sh | tee STATISTICS.md

    - Regenerate the Doxygen documentation
    - Push the Doxygen documentation to UH
    - Sync the repo with GitHub

## Profile Guided Optimization
Checkout [Profile-guided optimizations](https://learn.microsoft.com/en-us/cpp/build/profile-guided-optimizations?view=msvc-170) 

Target a build for the **Profile** configuration
  - Note:  This has been set with the following options:
    - C/C++ > Command Line > `/D PROFILE_GUIDED_OPTIMIZATION`
    - Linker > Optimization > Profile Guided Database > `$(OutDir)$(ProjectName)_$(PlatformTarget).pgd`
    - Linker > Optimization > Link Time Code Generation > Profile Guided Optimization - Instrument
    - Linker > Command Line > > `/genprofile:exact pgobootrun.lib`

Rebuild an executable as Profile - x64

From a PowerShell window, cd to the executable and run it

Open a Developer PowerShell, `cd` to the `%(SolutionDir)\x64\Profile` folder and 
run it from there.

Run DTMF Decoder and decode the following digits:  `1 5 9 D * 8 6 A 2 6 C 4 8 # 0 7 4 2 3 B C #` 

Note that several new *.pgc files are created.

Now, copy the profile-generated files `*.pgd` and `*.pgc` files to `$(SolutionDir)\Optimizer`

Target a build for the **Release** configuration
  - Note:  This has been set with the following options:
    - Linker > General > Suppress Startup Banner > No
    - Linker > Optimization > Profile Guided Database > `$(OutDir)$(ProjectName)_$(PlatformTarget).pgd`
    - Linker > Optimization > Link Time Code Generation > Program Guided Optimization - Optimize
    - Linker > Command Line > `/USEPROFILE:AGGRESSIVE`
    - Build Events > Pre-Build Event
      - Command line:  `copy $(SolutionDir)\Optimizer\* $(OutDir)`
      - Description:  `Copy PGO Instrumentation Files`

Clean the solution

Rebuild the solution as **Release**

Checkout the Release's profile database with:  `pgrmgr /summary .\DTMF_Decoder_x64_Release.pgd` and `pgomgr /summary /detail .\DTMF_Decoder_x64_Release.pgd`
