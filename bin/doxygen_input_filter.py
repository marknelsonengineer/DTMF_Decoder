#! python3

# #############################################################################
#          University of Hawaii, College of Engineering
#          DTMF_Decoder - EE 469 - Fall 2022
#
#  A Windows Desktop C program that decodes DTMF tones
#
## Preprocess source files for Doxygen
##
## 1.  Automatically generate the content in REFERENCES.md when it encounters
##     the keyword `<< Print All API Documentation >>`
## 2.  Automatically generate the API references in each source file when it
##     encounters the keyword `<< Print Module API Documentation >>`
## 3.  Automatically create links to the API references
##
## Enhance the source code's documentation without polluting it.  Say we have
## the following code snippet:
## ```
#       /// Use CloseHandle to close #ghStartDFTevent
#           br = CloseHandle( ghStartDFTevent );
#           CHECK_BR_R( IDS_GOERTZEL_FAILED_TO_CLOSE_STARTDFT_HANDLE );  //  "Failed to close ghStartDFTevent handle."
#           ghStartDFTevent = NULL;
## ```
##
## CloseHandle is a Windows API call documented at:  https://learn.microsoft.com/en-us/windows/win32/api/handleapi/nf-handleapi-closehandle
##
## This script will find comments that start with ///, then it will find
## API calls documented in an Excel file, finally, it will substitute the
## API call in the documentation with a Markdown link to the call in the form of:
## [dislayName](URL)
##
## @file    doxygen_input_filter.py
## @author  Mark Nelson <marknels@hawaii.edu>
###############################################################################

import sys    # For argv
import pandas # For better data processing

## Print without a \n and flush the buffer
#  @param aString The string to print
def printCat( aString ):
   ## Print a string
   #  @param end   The end character
   #  @param flush Flush the buffer
   print( aString, end='', flush=True )


## Print and flush the buffer
#  @param aString The string to print
def printFlush( aString ):
   ## Print a string
   #  @param flush Flush the buffer
   print( aString, flush=True )


## Do the string substitution
#  @param aString The string to perform the substitution on
#  @param xlsData A dataset that can be interated over containing the
#                 approproate documentation
def documentKeywords( aString, xlsData ):
   for index, row in xlsData.iterrows():
      aString = aString.replace( row['Name'], f"[{row['Name']}]({row['URL']})" )
   return aString


# The main body

## The source filename
sourceFilename = sys.argv[1]

## Handle to the source file
sourceFile = open( sourceFilename, 'r' )


# Don't do API replacement for Python source files
if sourceFilename[-3:] == ".py":
   for line in sourceFile:
      printCat( line )
   exit( 0 )


## Path to the API Excel file
API_Documentation = "API_Documentation.xls"

## DataFrame from Pandas Excel API
xlsData = pandas.read_excel( API_Documentation )
xlsData.sort_values( ['Section', 'Name'],
                     ## Leave the results in self (xlsData)
                     inplace=True
                   )

# print( "Processing file [" + sys.argv[1] + "] using the API references in [" + API_Documentation + "]", file=sys.stderr )



## Iterate over sourceFile, putting each line in `line`
for line in sourceFile:

   # Process the documentation for REFERENCES.md
   hasTag = line.find( "<< Print All API Documentation >>" )
   if( hasTag != -1 ):
      ## Track the section and print headers when it changes
      section=""
      for index, row in xlsData.iterrows():
         if( section != row['Section'] ):
            print( "" )
            print( "## " + row['Section'] )
            print( "| API | Link |" )
            print( "|-----|------|" )
            section = row['Section']
         printFlush( f"| [{row['Name']}]({row['URL']}) | {row['URL']} |" )
      printFlush( "" )
      continue

   # Process the documentation for a module's API section
   hasTag = line.find( "<< Print Module API Documentation >>" )
   if( hasTag != -1 ):
      ## The set of keywords in the source file
      keywordSet = set()
      ## Scan through the source file a second time (looking for API calls)
      sourceFile_2 = open( sourceFilename, 'r' )
      for line_2 in sourceFile_2:
         for index, row in xlsData.iterrows():
            start = line_2.find( row['Name'] )
            if( start != -1 ):
               # print( f"Found {row['Name']}" )
               keywordSet.add( row['Name'] )

      # print( keywordSet )

      section = ""
      for index, row in xlsData.iterrows():
         if row['Name'] not in keywordSet:
            continue
         if( section != row['Section'] ):
            print( "/// " )
            print( "/// #### " + row['Section'] )
            print( "/// | API | Link |" )
            print( "/// |-----|------|" )
            section = row['Section']
         printFlush( f"/// | [{row['Name']}]({row['URL']}) | {row['URL']} |" )
      printFlush( "" )
      continue

   # Automatically create links to the API references

   ## Find the start of a DOxygen comment `///`
   start = line.find( "///" )

   if( start == -1 ):
      printCat( line )
      continue

   ## The source before the DOxygen comment
   preComment = line[0:start]

   ## The Doxygen comment's contents
   postComment = line[start+3:]

   postComment = documentKeywords( postComment, xlsData )

   printCat( f"{preComment}///{postComment}" )
