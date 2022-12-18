@echo off

:: #############################################################################
:: #          University of Hawaii, College of Engineering
:: #          DTMF_Decoder - EE 469 - Fall 2022
:: #
:: #  A Windows Desktop C program that decodes DTMF tones
:: #
:: ## Call a Python script that preprocesses source files for Doxygen
:: ##
:: ## @file    doxygen_input_filter.bat
:: ## @author  Mark Nelson <marknels@hawaii.edu>
:: #############################################################################

python ./bin/doxygen_input_filter.py %1
