###############################################################################
##          University of Hawaii, College of Engineering
##          DTMF_Decoder - EE 469 - Fall 2022
##
##  A Windows Desktop C program that decodes DTMF tones
##
### Run before building code in Visual Studio
###
### @file    stats.sh
### @version 2.0
###
### @author  Mark Nelson <marknels@hawaii.edu>
### @date    7_Nov_2022
###############################################################################

param ($Configuration, $Platform, $SolutionDir, $OutDir)

Write-Host "Configuration: $Configuration"
Write-Host "Platform: $Platform"
Write-Host "SolutionDir: $SolutionDir"
Write-Host "OutDir: $OutDir"

."$SolutionDir\bin\update_version.ps1" "$SolutionDir\DTMF_Decoder\version.h"
