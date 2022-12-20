#! python3

###############################################################################
#          University of Hawaii, College of Engineering
#          DTMF_Decoder - EE 469 - Fall 2022
#
#  A Windows Desktop C program that decodes DTMF tones
#
## Preprocess source files for Doxygen
## 
## 1.  Automatically generate the content in REFERENCES.md
## 2.  Automatically generate the API references in each source file
## 3.  Automatically create links back to the API references
##
## Enhance the source code's documentation without polluting it.  Say we have
## the following code snippet:
##      /// Use CloseHandle to close #ghStartDFTevent
##         br = CloseHandle( ghStartDFTevent );
##         CHECK_BR_R( IDS_GOERTZEL_FAILED_TO_CLOSE_STARTDFT_HANDLE );  //  "Failed to close ghStartDFTevent handle."
##         ghStartDFTevent = NULL;
##
## CloseHandle is a Windows API call documented at:  https://learn.microsoft.com/en-us/windows/win32/api/handleapi/nf-handleapi-closehandle
##
## This script will find comments that start with ///, then it will find
## API calls documented in a .CSV file, finally, it will substitute the
## API call in the documentation with a Markdown link to the call in the form of:
## [dislayName](URL)
##
## @file    doxygen_input_filter.py
## @author  Mark Nelson <marknels@hawaii.edu>
###############################################################################

import os     # For splittext
import sys    # For argv
import csv    # For comma separated list processing
import re     # Regular expressions
import pandas # For better CSV processing
#import codecs # For setting the writer to UTF8

#sys.stdout = codecs.getwriter('utf8')(sys.stdout)
#sys.stderr = codecs.getwriter('utf8')(sys.stderr)
#sys.stdout.reconfigure(encoding='utf8')
#sys.stdout = open(sys.stdout.fileno(), mode='w', encoding='utf8', buffering=1)



# The main body

API_Documentation = "API_Documentation.csv"

# print( "Processing file [" + sys.argv[1] + "] using the API references in [" + API_Documentation + "]", file=sys.stderr )


sourceFile = open( sys.argv[1], 'r' )
for line in sourceFile:

   # Process the documentation for REFERENCES.md
   hasTag = line.find( "<< Print All API Documentation >>" )
   if( hasTag != -1 ):
      csvData = pandas.read_csv( API_Documentation )
      csvData.sort_values( ['Section', 'Function'], inplace=True )
      section=""
      for index, row in csvData.iterrows():
         if( section != row['Section'] ):
            print( "" )
            print( "## " + row['Section'] )
            print( "| API | Link |" )
            print( "|-----|------|" )
            section = row['Section']
         print( f"| [{row['Function']}]({row['URL']}) | {row['URL']} |" )
      print( "" )
      continue

   # Process the documentation for a module's API section
   hasTag = line.find( "<< Print Module API Documentation >>" )
   if( hasTag != -1 ):
      keywordSet = set()
      sourceFile_2 = open( sys.argv[1], 'r' )
      for line_2 in sourceFile_2:
         csvData = pandas.read_csv( API_Documentation )
         for index, row in csvData.iterrows():
            start = line_2.find( row['Function'] )
            if( start != -1 ):
               # print( f"Found {row['Function']}" )
               keywordSet.add( row['Function'] )

      # print( keywordSet )

      csvData = pandas.read_csv( API_Documentation )
      csvData.sort_values( ['Section', 'Function'], inplace=True )
      section = ""
      for index, row in csvData.iterrows():
         if row['Function'] not in keywordSet:
            continue
         if( section != row['Section'] ):
            print( "/// " )
            print( "/// #### " + row['Section'] )
            print( "/// | API | Link |" )
            print( "/// |-----|------|" )
            section = row['Section']
         print( f"/// | [{row['Function']}]({row['URL']}) | {row['URL']} |" )
      print( "" )
      continue

   start = line.find( "///" )

   if( start == -1 ):
      print( line, end='', flush=True )
      continue

   preComment = line[0:start]
   postComment = line[start+3:]

   csvFile = open( API_Documentation, newline='' )
   reader = csv.DictReader( csvFile )
   for row in reader:
      postComment = postComment.replace( row['Function'], f"[{row['Function']}]({row['URL']})" )

   print( preComment, end='' )
   print( "///", end='' )
   print( postComment, end='', flush=True )
