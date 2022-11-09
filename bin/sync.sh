##          University of Hawaii, College of Engineering
##          DTMF_Decoder - EE 469 - Fall 2022
##
##  A Windows Desktop C program that decodes DTMF tones
##
### Synchronize the local Doxygen directory with UHUnix
###
### @file    sync.sh
### @author  Mark Nelson <marknels@hawaii.edu>
###############################################################################

rsync --recursive --checksum --delete --compress --stats ~/src/VisualStudio/DTMF_Decoder/Doxygen/html/ marknels@uhunix.hawaii.edu:~/public_html/DTMF_Decoder
