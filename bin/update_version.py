#! python3

###############################################################################
#          University of Hawaii, College of Engineering
#          DTMF_Decoder - EE 469 - Fall 2022
#
#  A Windows Desktop C program that decodes DTMF tones
#
## Increment the build number in versioh.h and other files
##
## Usage:  Normally, this script just updates the `version.h` file.  However,
##         if you run it with the `--all` command line option, it will update:
##         - version.h
##         - The VERSION object in resource.rc
##         - The Doxyfile
##         - The Visual Studio project configuration file
##
## @see https://stackoverflow.com/questions/59692711/auto-increment-fileversion-build-nr-in-visual-studio-2019
##
## @file    update_version.py
## @author  Mark Nelson <marknels@hawaii.edu>
###############################################################################

import datetime
import argparse

## Path to version.h in C
VERSION_HEADER_FILE = "./DTMF_Decoder/version.h"

## Path to the Doxygen configuration file
DOXYGEN_CONFIG_FILE = "./Doxyfile"

## Path to the program's resource file
RESOURCE_FILE       = "./DTMF_Decoder/DTMF_Decoder.rc"

## Path to the project's configuration file
VCXPROJ_FILE        = "./DTMF_Decoder/DTMF_Decoder.vcxproj"

## Increments with major functional changes
major_version = 0

## Increments with minor functional changes and bugfixes
minor_version = 0

## Increments with bugfixes
patch_version = 0

## Monotonic counter that tracks the number of compilations
build_version = 0


# print("Starting update_version.py")

##\cond
parser = argparse.ArgumentParser(prog='update_version.py', description='Update version numbers for a Visual Studio project.')
parser.add_argument( '--all', action='store_true', help='Update all files (not just version.h)')
args = parser.parse_args()
# parser.print_help()
##\endcond


## Extract an integer from a line in a file
##
## If the source file had a like like:
##
## `#define VERSION_MINOR    4`
##
## then
##
## `extractInt( "#define VERSION_MINOR", aLine)` would return `4` as an `int`.
##
def extractInt( sKey:str, sLine:str ) -> int:
   i = sLine.find( sKey )  # Find the leading string

   if( i == -1 ):          # If not found, return -1
      return -1

   i2 = i + len( sKey )    # Get the remaining part of the string

   i3 = int( sLine[i2:] )  # Convert it to an int

   return i3


## Get the full version number (as a string) from `version.h`
##
## @returns A string like `1.4.0.2202`
def getFullVersion() -> str:
   global major_version
   global minor_version
   global patch_version
   global build_version

   with open ( VERSION_HEADER_FILE, "rt" ) as versionFile:  # open for reading text
      for aLine in versionFile:              # For each line, read to a string,
         # print(myline)                   # and print the string.
         i = extractInt( "#define VERSION_MAJOR", aLine )
         if i != -1:
            major_version = i

         i = extractInt( "#define VERSION_MINOR", aLine )
         if i != -1:
            minor_version = i

         i = extractInt( "#define VERSION_PATCH", aLine )
         if i != -1:
            patch_version = i

         i = extractInt( "#define VERSION_BUILD", aLine )
         if i != -1:
            build_version = i

   full_version = str(major_version)         \
                + "." + str( minor_version ) \
                + "." + str( patch_version ) \
                + "." + str( build_version ) \

   return( full_version )


## Update the build line in `version.h`
##
## If the old build line was: `#define VERSION_BUILD 1045`
##
## Then the new build line will be:  `#define VERSION_BUILD 1046`
##
## This routine rewrites `version.h`
def updateVersion( sKey:str, sFilename:str ):
   li = []

   with open( sFilename, "rt") as versionFile:
      for aLine in versionFile:
         i = aLine.find( sKey )
         if i >= 0:
            oldBuild = extractInt( sKey, aLine )
            newBuild = oldBuild + 1
            newLine = aLine.replace( str(oldBuild), str(newBuild) )
            li.append( newLine )
         else:
            li.append( aLine )

   j = 0
   with open( sFilename, "wt" ) as versionFile:
      while j < len(li):
         versionFile.write( li[j] )
         j += 1


## Update the PROJECT_NUMBER field in `Doxyfile`
##
## We expect the configuration like to look like this:
##
## `PROJECT_NUMBER         = 1.4.0.1044`
##
## This routine rewrites `Doxyfile` updating the PROJECT_NUMBER to the current
## version.
##
## This routine should only run when `update_version.py` is run with
## the `--all` command line option.
def updateDoxygenConfigVersion( sKey:str, sFilename:str, sVersion:str ):
   li = []

   with open( sFilename, "rt") as versionFile:
      for aLine in versionFile:
         if aLine.startswith( sKey ):
            li.append( sKey + " = " + sVersion + '\n' )
         else:
            li.append( aLine )

   j = 0
   with open( sFilename, "wt" ) as versionFile:
      while j < len(li):
         versionFile.write( li[j] )
         j += 1


## Update several fields in `Resource.rc`
##
## We expect the configuration like to look like this:
##
## @verbatim
##    VS_VERSION_INFO VERSIONINFO
##     FILEVERSION 1,4,0,1044
##     PRODUCTVERSION 1,4,0,1044
##    ...
##            BEGIN
##                VALUE "CompanyName", "Mark Nelson"
##                VALUE "FileDescription", "A Windows desktop program that decodes DTMF tones"
##                VALUE "FileVersion", "1.4.0.1044"
##                VALUE "InternalName", "DTMF_Dec.exe"
##                VALUE "LegalCopyright", "Copyright (C) 2022, Mark Nelson"
##                VALUE "OriginalFilename", "DTMF_Dec.exe"
##                VALUE "ProductName", "DTMF Decoder"
##                VALUE "ProductVersion", "1.4.0.1044"
##            END
## @endverbatim
##
## This routine rewrites `Resource.rc` updating the following fields:
##   - `FILEVERSION`
##   - `PRODUCTVERSION`
##   - `"FileVersion"`
##   - `"ProductVersion"`
##   - `"LegalCopyright"`
##
## This routine should only run when `update_version.py` is run with
## the `--all` command line option.
def updateResourceFileConfigVersion( sFilename:str, sVersion:str ):
   li = []

   with open( sFilename, "r", encoding='utf-16-le') as versionFile:
      for aLine in versionFile:
         sKey1 = "VALUE \"FileVersion\","
         i1 = aLine.find( sKey1 )

         sKey2 = "VALUE \"ProductVersion\","
         i2 = aLine.find( sKey2 )

         sKey3 = "VALUE \"LegalCopyright\","
         i3 = aLine.find( sKey3 )

         sKey4 = " FILEVERSION "
         i4 = aLine.find( sKey4 )

         sKey5 = " PRODUCTVERSION "
         i5 = aLine.find( sKey5 )

         if i1 != -1:
            li.append( aLine[0:i1] + sKey1 + " \"" + sVersion + "\"" + '\n' )
         elif i2 != -1:
            li.append( aLine[0:i2] + sKey2 + " \"" + sVersion + "\"" + '\n' )
         elif i3 != -1:
            li.append( aLine[0:i3] + sKey3 + " \"Copyright (C) " + str(datetime.date.today().year) + ", Mark Nelson\"" + '\n' )
         elif i4 != -1:
            li.append( aLine[0:i4] + sKey4 + str( major_version ) + ","\
                                           + str( minor_version ) + ","\
                                           + str( patch_version ) + ","\
                                           + str( build_version ) + '\n' )
         elif i5 != -1:
            li.append( aLine[0:i5] + sKey5 + str( major_version ) + ","\
                                           + str( minor_version ) + ","\
                                           + str( patch_version ) + ","\
                                           + str( build_version ) + '\n' )
         else:
            li.append( aLine )

   j = 0
   with open( sFilename, "w", encoding='utf-16-le' ) as versionFile:
      while j < len(li):
         versionFile.write( li[j] )
         j += 1



## Update the `<Version>` key in a Visual Studio project configuration file
##
## We expect the configuration like to look like this:
##
## `      <Version>1.4</Version>`
##
## Note:  The PE File Format only accepts a major_version and minor_version.
##
## This routine rewrites the XML config file updating `<Version>` to
## the current version in multiple locations.
##
## This routine should only run when `update_version.py` is run with
## the `--all` command line option.
def updateVcxprojVersion( sKey:str, sFilename:str, sVersion:str ):
   li = []

   with open( sFilename, "rt") as versionFile:
      for aLine in versionFile:
         i1 = aLine.find( sKey )
         if i1 != -1:
            li.append( aLine[0:i1] + sKey + str(major_version) \
                                    + "." + str(minor_version) \
                                    + "</Version>" + '\n' )
         else:
            li.append( aLine )

   j = 0
   with open( sFilename, "wt" ) as versionFile:
      while j < len(li):
         versionFile.write( li[j] )
         j += 1


# The main body of the program
updateVersion( "#define VERSION_BUILD", VERSION_HEADER_FILE )

## Holds the full version as a string: `1.4.0.2022`
full_version = getFullVersion()

if args.all:
   print( "Updating version in all files" )

   updateDoxygenConfigVersion( "PROJECT_NUMBER", DOXYGEN_CONFIG_FILE, full_version )
   updateResourceFileConfigVersion( RESOURCE_FILE, full_version )
   updateVcxprojVersion( "<Version>", VCXPROJ_FILE, full_version )

print( "Build: " + full_version )
