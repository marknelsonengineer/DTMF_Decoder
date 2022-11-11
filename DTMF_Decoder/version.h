///////////////////////////////////////////////////////////////////////////////
//          University of Hawaii, College of Engineering
//          DTMF_Decoder - EE 469 - Fall 2022
//
//  A Windows Desktop C program that decodes DTMF tones
//
/// Program version and build tracker
///
/// @see https://stackoverflow.com/questions/59692711/auto-increment-fileversion-build-nr-in-visual-studio-2019
///
/// @file    version.h
/// @author  Mark Nelson <marknels@hawaii.edu>
///////////////////////////////////////////////////////////////////////////////

#pragma once

/// Increments with major functional changes
#define VERSION_MAJOR    1

/// Increments with minor functional changes and bugfixes
#define VERSION_MINOR    4

/// Increments with bugfixes
#define VERSION_PATCH    0

/// Monotonic counter that tracks the number of compilations
#define VERSION_BUILD 1106

/// C preprocesor trick that converts values into strings at compile time
/// @see https://stackoverflow.com/questions/12844364/stringify-c-preprocess
#define stringify(a)  stringify_(a)

/// Second step of the stringify process
#define stringify_(a) #a

/// The full version number as a string
#define FULL_VERSION     stringify( VERSION_MAJOR ) \
                     "." stringify( VERSION_MINOR ) \
                     "." stringify( VERSION_PATCH ) \
                     "." stringify( VERSION_BUILD )
