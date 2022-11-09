###############################################################################
#          University of Hawaii, College of Engineering
#          DTMF_Decoder - EE 469 - Fall 2022
#
#  A Windows Desktop C program that decodes DTMF tones
#
## Script a number of activities before the build runs
##
## @file    pre_build_event.py
## @version 1.0
##
## @author  Mark Nelson <marknels@hawaii.edu>
## @date    9_Nov_2022
###############################################################################

import os
import sys
import glob
import shutil

# print( "Starting " + os.path.basename(__file__) )

Configuration = sys.argv[1]
Platform      = sys.argv[2]
SolutionDir   = sys.argv[3]
OutDir        = sys.argv[4]

# print( "Configuration: " + Configuration )
# print( "Platform: "      + Platform )
# print( "SolutionDir: "   + SolutionDir )
# print( "OutDir: "        + OutDir )

os.chdir( SolutionDir )

os.system( "python3 " + SolutionDir + "bin/update_version.py" )

if Configuration == "Release":
    print( "Copying PGO instrumentation files")
    for file in glob.glob( SolutionDir + "Optimizer/*" ):
        shutil.copy( file, OutDir )
