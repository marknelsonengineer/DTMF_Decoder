#! python3
# @todo Header

import datetime

VERSION_HEADER_FILE = "./DTMF_Decoder/version.h"
DOXYGEN_CONFIG_FILE = "./Doxyfile"
RESOURCE_FILE       = "./DTMF_Decoder/DTMF_Decoder.rc"
VCXPROJ_FILE        = "./DTMF_Decoder/DTMF_Decoder.vcxproj"

major_version = 0
minor_version = 0
patch_version = 0
build_version = 0


print("Starting update_version.py")


def extractInt( sKey:str, sLine:str ) -> int:
	i = sLine.find( sKey )  # Find the leading string

	if( i == -1 ):          # If not found, return -1
	   return -1

	i2 = i + len( sKey )    # Get the remaining part of the string

	i3 = int( sLine[i2:] )  # Convert it to an int

	return i3


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


def updateDoxygenConfigVersion( sKey:str, sFilename:str, sVersion:str ):
   li = []

   with open( sFilename, "rt") as versionFile:
      for aLine in versionFile:
         if aLine.startswith( sKey ):
            li.append( sKey + " = " + sVersion + '\n' + '\n' )
         else:
            li.append( aLine )

   j = 0
   with open( sFilename, "wt" ) as versionFile:
      while j < len(li):
         versionFile.write( li[j] )
         j += 1


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


def updateVcxprojVersion( sKey:str, sFilename:str, sVersion:str ):
   li = []

   with open( sFilename, "rt") as versionFile:
      for aLine in versionFile:
         i1 = aLine.find( sKey )
         if i1 != -1:
            li.append( aLine[0:i1] + sKey + sVersion + "</Version>" + '\n' )
         else:
            li.append( aLine )

   j = 0
   with open( sFilename, "wt" ) as versionFile:
      while j < len(li):
         versionFile.write( li[j] )
         j += 1


updateVersion( "#define VERSION_BUILD", VERSION_HEADER_FILE )
full_version = getFullVersion()
updateDoxygenConfigVersion( "PROJECT_NUMBER", DOXYGEN_CONFIG_FILE, full_version )
updateResourceFileConfigVersion( RESOURCE_FILE, full_version )
updateVcxprojVersion( "<Version>", VCXPROJ_FILE, full_version )

print( full_version )
