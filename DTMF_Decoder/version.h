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
/// @version 1.0
///
/// @author  Mark Nelson <marknels@hawaii.edu>
/// @date    7_Nov_2022
///////////////////////////////////////////////////////////////////////////////

#pragma once

/// Increments with major functional changes
#define VERSION_MAJOR    1

/// Increments with minor functional changes and bugfixes
#define VERSION_MINOR    4

/// Increments with bugfixes
#define VERSION_PATCH    0

/// Monotonic counter that tracks the number of compilations
#define VERSION_BUILD 1046

#define stringify(a)  stringify_(a)
#define stringify_(a) #a
