Release Checklist
=================

This checklist is for a Visual Studio Win32 C Windows program

- For each source file:
    - Cleanup `#include` files
    - Ensure all parameters are validated with `_ASSERTE`
    - Ensure all parameters have [SAL](https://learn.microsoft.com/en-us/cpp/code-quality/using-sal-annotations-to-reduce-c-cpp-code-defects?view=msvc-170) annotations
    - Look for `const`-able parameters
    - Look for file-local functions that should be declared `static`
    - Is everything documented?
    - No Code Analysis or Clang-Tidy warnings

- Doxygen
    - No Doxygen warnings
    - Proofread the Doxygen-generated content

- Last looks
    - Read through all of the `@todo`s
    - Switch to **Analyze** Configuration and `Rebuild Project`.  Accept any warnings.
    - Run `update_version.py --all` from `$(SolutionDir)`
      - This script updates the version number in:
        - Doxygen
        - Release > Linker > General > Version
        - Resource file > About dialog box
        - Resource file > Version
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
    - Regenerate the statistics by clicking the Stats button or running:

          wsl --cd ~/src/VisualStudio/DTMF_Decoder --exec ./bin/stats.sh

    - Regenerate the Doxygen documentation
    - Push the Doxygen documentation to UH from WSL's `bash` `cd $(SolutionDir)` `./bin/sync.sh`
    - Sync the repo with GitHub

## Profile Guided Optimization
Checkout [Profile-guided optimizations](https://learn.microsoft.com/en-us/cpp/build/profile-guided-optimizations?view=msvc-170) 

Target a build for the **Profile** configuration
  - Note:  This has been set with the following options:
    - C/C++ > Command Line > `/D PROFILE_GUIDED_OPTIMIZATION` (Note: This is not currently used)
    - Linker > Optimization > Profile Guided Database > `$(OutDir)$(ProjectName)_$(PlatformTarget).pgd` (Remove Configuration from the filename)
    - Linker > Optimization > Link Time Code Generation > Profile Guided Optimization - Instrument
    - Linker > Command Line > > `/GENPROFILE:EXACT pgobootrun.lib`

Rebuild an executable as **Profile - x64**

Open a Developer PowerShell (or x86 Native Tools Command Prompt), `cd` to 
the `%(SolutionDir)\x64\Profile` folder and run it from there.  
Usually, I'll delete all of the files in this directory,
do a final Rebuild and then do a final profile run of the application.

Run DTMF Decoder and decode the following digits:  `1 5 9 D * 8 6 A 2 6 C 4 8 # 0 7 4 2 3 B C #` 

Note that a new `*.pgc` file is created.  After a good profile run, copy the 
profile-generated files `*.pgd` and `*.pgc` files to `$(SolutionDir)\Optimizer`

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
