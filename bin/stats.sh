#!/bin/bash

# @todo header
# @todo get into associative arrays for passing data around
# @todo gcc -fpreprocessed does not work on the mac


ALL_FILES=$(mktemp -q /tmp/stats.XXXXXX || exit 1)
WORKING_FILES=$(mktemp -q /tmp/stats.XXXXXX || exit 1)
REMAINING_FILES=$(mktemp -q /tmp/stats.XXXXXX || exit 1)

# Set trap to clean up the temporary files
trap 'rm -f -- "$ALL_FILES" "$WORKING_FILES" "$REMAINING_FILES"' EXIT


# Put all of the source files in $ALL_FILES
find . ! -path "./Doxygen/*" ! -path "./x64/*" ! -path "*/.vs/*" ! -path "./.git/*" ! -path "*/Release/*" ! -path "*/Debug/*" ! -path "./Ghidra/*" ! -name ".DS_Store" -type f > $ALL_FILES


function print_title {
  printf "Project Statistics\n"
  printf "==================\n"
  printf "\n"
}


function processSourceFiles {
   rowHeader=("$1")
   shift

   arr=("$@")
   file_count=0
   raw_lines=0
   raw_bytes=0
   working_lines=0
   working_bytes=0

   for i in "${arr[@]}"; do
      # processFile "$i"
      ((file_count+=1))
      ((raw_lines+=`wc -l "$i" | awk '{print $1}'`))
      ((raw_bytes+=`wc -c "$i" | awk '{print $1}'`))
      ((working_lines+=`gcc -fpreprocessed -dD -E "$i" 2> /dev/null | sed 1,2d | grep "\S" | wc -l | awk '{print $1}'`))
      ((working_bytes+=`gcc -fpreprocessed -dD -E "$i" 2> /dev/null | sed 1,2d | grep "\S" | wc -c | awk '{print $1}'`))
   done

   printf "|%s|%'d|%'d|%'d|%'d|%'d|\n" "$rowHeader" "$file_count" $raw_lines $raw_bytes $working_lines $working_bytes
}


function processTextFiles {
   rowHeader=("$1")
   shift

   arr=("$@")
   file_count=0
   raw_lines=0
   raw_bytes=0

   for i in "${arr[@]}"; do
      # processFile "$i"
      ((file_count+=1))
      ((raw_lines+=`wc -l "$i" | awk '{print $1}'`))
      ((raw_bytes+=`wc -c "$i" | awk '{print $1}'`))
   done

   printf "|%s|%'d|%'d|%'d|n/a|n/a|\n" "$rowHeader" "$file_count" $raw_lines $raw_bytes
}


function processDataFiles {
   rowHeader=("$1")
   shift

   arr=("$@")
   file_count=0
   raw_bytes=0

   for i in "${arr[@]}"; do
      # processFile "$i"
      ((file_count+=1))
      ((raw_bytes+=`wc -c "$i" | awk '{print $1}'`))
   done

   printf "|%s|%'d|n/a|%'d|n/a|n/a|\n" "$rowHeader" "$file_count" $raw_bytes
}


function process_row_as_source {
   array=()
   while IFS=  read -r -d $'\0'; do
      array+=("$REPLY")
   done < <(eval "${2} -print0")
   processSourceFiles "${1}" "${array[@]}"
}


function process_row_as_text {
   array=()
   while IFS=  read -r -d $'\0'; do
      array+=("$REPLY")
   done < <(eval "${2} -print0")
   processTextFiles "${1}" "${array[@]}"
}


function process_row_as_data {
   array=()
   while IFS=  read -r -d $'\0'; do
      array+=("$REPLY")
   done < <(eval "${2} -print0")
   processDataFiles "${1}" "${array[@]}"
}


function extract_working_files {
   egrep    ${1}  $ALL_FILES > $WORKING_FILES
   egrep -v ${1}  $ALL_FILES > $REMAINING_FILES
   mv $REMAINING_FILES $ALL_FILES
}


function print_file_statistics {
   printf "| Type         | Files | Lines | Bytes | Working Lines | Working Bytes |\n"
   printf "|--------------|------:|------:|------:|--------------:|--------------:|\n"
   extract_working_files "\.h$"
   processSourceFiles '.h' `cat $WORKING_FILES`

   extract_working_files "\.cpp$"
   processSourceFiles '.cpp Sources' `cat $WORKING_FILES`

   extract_working_files "\.md$"
   processTextFiles '.md' `cat $WORKING_FILES`
   
   extract_working_files "\.svg$|\.png$|\.jpg$|\.jpeg$|\.ico$"
   processDataFiles 'Images' `cat $WORKING_FILES`
   
   processDataFiles 'Other Files' `cat $ALL_FILES`
}


function print_number_of_unit_tests {
   printf "\n"
   unit_tests=`grep "BOOST_.*_TEST_CASE" ../tests/* | wc -l | awk '{print $1}'`
   printf "Number of unit tests:  %d\n" $unit_tests
}


function print_number_of_individual_tests {
   printf "\n"
   unit_tests=`egrep "BOOST_CHECK|BOOST_REQUIRE" ../tests/* | wc -l | awk '{print $1}'`
   printf "Number of individual tests:  %d\n" $unit_tests
}


function print_number_of_commits {
   printf "\n"
   unit_tests=`git rev-list --all --count`
   printf "Number of commits:  %d\n" $unit_tests
}


function print_date {
   printf "\n"
   printf "Automatically generated on %s\n" "`date`"
}


print_title
print_file_statistics
# print_number_of_unit_tests
# print_number_of_individual_tests
# print_number_of_commits  # Broken because remote directory is not a git repo
print_date


# Use the following to get the tag/release history
# $ git tag --sort=v:refname --format="%(tag)|%(contents)|%(creator)"


# clean up the temporary files
rm -f -- "$ALL_FILES" "$WORKING_FILES" "$REMAINING_FILES"
trap - EXIT
exit
