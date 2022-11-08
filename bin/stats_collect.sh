#!/bin/bash

###############################################################################
##          University of Hawaii, College of Engineering
##          DTMF_Decoder - EE 469 - Fall 2022
##
##  A Windows Desktop C program that decodes DTMF tones
##
### Print project statistics to stdout
###
### @see https://shields.io
###
### @todo get into associative arrays for passing data around
### @todo gcc -fpreprocessed does not work on the mac
###
### @file    stats_collect.sh
### @version 2.0
###
### @author  Mark Nelson <marknels@hawaii.edu>
### @date    4_Nov_2022
###############################################################################


# Create 3 temporary files
ALL_FILES=$(mktemp -q /tmp/stats.XXXXXX || exit 1)
WORKING_FILES=$(mktemp -q /tmp/stats.XXXXXX || exit 1)
REMAINING_FILES=$(mktemp -q /tmp/stats.XXXXXX || exit 1)


# Trap to clean up temporary files
trap 'rm -f -- "$ALL_FILES" "$WORKING_FILES" "$REMAINING_FILES"' EXIT


function get_all_files {
	# Put all of the source files in $ALL_FILES
	find . ! -path "./Doxygen/*" ! -path "./x64/*" ! -path "*/.vs/*" ! -path "./.git/*" ! -path "*/Release/*" ! -path "*/Debug/*" ! -path "*/Profile/*" ! -path "./Ghidra/*" ! -name ".DS_Store" -type f > $ALL_FILES
}


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
   number_of_commits=`git rev-list --all --count`
   printf "Number of commits:  %d\n\n" $number_of_commits
}


function print_date {
   printf "\n"
   printf "Automatically generated on %s\n" "`date`"
}


function print_shields_io_tags {
   GITHUB_USERNAME="marknelsonengineer"
	GITHUB_REPO="DTMF_Decoder"
	STYLE="social"

	repo1="![GitHub code size in bytes](https://img.shields.io/github/languages/code-size/${GITHUB_USERNAME}/${GITHUB_REPO}?style=${STYLE})"
	repo2="![GitHub repo size](https://img.shields.io/github/repo-size/${GITHUB_USERNAME}/${GITHUB_REPO}?style=${STYLE})"
	repo3="![GitHub contributors](https://img.shields.io/github/contributors/${GITHUB_USERNAME}/${GITHUB_REPO}?style=${STYLE})"

	commit1="![GitHub commit activity](https://img.shields.io/github/commit-activity/w/${GITHUB_USERNAME}/${GITHUB_REPO}?style=${STYLE})"
	commit2="![GitHub last commit](https://img.shields.io/github/last-commit/${GITHUB_USERNAME}/${GITHUB_REPO}?style=${STYLE})"
	issue1="![GitHub issues](https://img.shields.io/github/issues-raw/${GITHUB_USERNAME}/${GITHUB_REPO}?style=${STYLE})"
	issue2="![GitHub closed issues](https://img.shields.io/github/issues-closed-raw/${GITHUB_USERNAME}/${GITHUB_REPO}?style=${STYLE})"
	social1="![GitHub forks](https://img.shields.io/github/forks/${GITHUB_USERNAME}/${GITHUB_REPO}?style=${STYLE})"
	social2="![GitHub Repo stars](https://img.shields.io/github/stars/${GITHUB_USERNAME}/${GITHUB_REPO}?style=${STYLE})"

	printf "\n"
	printf "## GitHub Statistics\n"

	print_number_of_commits

   printf "| Repository                            | Commits                     | Issues                    | Social                      |\n"
   printf "|---------------------------------------|-----------------------------|---------------------------|-----------------------------|\n"
   printf "|${repo1} <br/> ${repo2} <br/> ${repo3} | ${commit1} <br/> ${commit2} | ${issue1} <br/> ${issue2} | ${social1} <br/> ${social2} |\n"
}


function print_tags {
# Use the following to get the tag/release history
# $ git tag --sort=v:refname --format="%(tag)|%(contents)|%(creator)"
	printf "\n"
	printf "## Tags\n"

	printf "| Tag | Date | Author |\n"
	printf "|-----|------|--------|\n"
	git for-each-ref --sort=creatordate --format '|%(refname:strip=2)|%(creatordate)|%(authorname)|' refs/tags

	printf "\n"
}


print_title
get_all_files
print_file_statistics
# print_number_of_unit_tests
# print_number_of_individual_tests
# print_number_of_commits  # Broken because remote directory is not a git repo
print_tags
print_shields_io_tags
print_date


# clean up the temporary files
rm -f -- "$ALL_FILES" "$WORKING_FILES" "$REMAINING_FILES"
trap - EXIT
exit
