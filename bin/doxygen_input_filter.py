#! python3

###############################################################################
#          University of Hawaii, College of Engineering
#          DTMF_Decoder - EE 469 - Fall 2022
#
#  A Windows Desktop C program that decodes DTMF tones
#
## Preprocess source files for Doxygen
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

# The main body

API_Documentation = "API_Documentation.csv"

# print( "Processing file [" + sys.argv[1] + "] using the API references in [" + API_Documentation + "]", file=sys.stderr )

fileSplit = os.path.splitext( sys.argv[1] )

sourceFile = open( sys.argv[1], 'r' )
for line in sourceFile:

   csvFile = open( API_Documentation, newline='' )
   reader = csv.DictReader( csvFile )
   for row in reader:
      x = re.search( f"\|.*{row['Function']}.*\|", line )
      if( x != None ):
         line= f"{line[0:x.start()]}| {row['Function']} | {row['URL']} |\n"
         continue
	
	# Process the whole line if it's a markdown file
   if( fileSplit[1] == ".md" ):
      csvFile = open( API_Documentation, newline='' )
      reader = csv.DictReader( csvFile )
      for row in reader:
         line = line.replace( row['Function'], f"[{row['Function']}]({row['URL']})" )
   
      print( line, end='', flush=True )
      continue

	# Otherwise, this is not an MD file, so only process the /// comments
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
