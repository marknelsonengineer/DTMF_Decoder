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

import argparse
import os

print( "Starting " + os.path.basename(__file__) )

##\cond
parser = argparse.ArgumentParser(prog='pre_build_event.py', description='Pre-Build Event script for Visual Studio.')
parser.add_argument('Configuration')      # positional argument
parser.add_argument('Platform')           # positional argument
parser.add_argument('SolutionDir')        # positional argument
parser.add_argument('OutDir')             # positional argument
args = parser.parse_args()
##\endcond


