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
#define VERSION_BUILD 2044

#ifndef STRINGIFY_VALUE
/// C preprocesor trick that converts values into strings at compile time
/// @see https://stackoverflow.com/questions/12844364/stringify-c-preprocess
#define STRINGIFY_VALUE(a)  STRINGIFY_VALUE_(a)

/// Second step of the stringify process
#define STRINGIFY_VALUE_(a) #a
#endif

/// The full version number as a narrow string
#define FULL_VERSION     STRINGIFY_VALUE( VERSION_MAJOR ) \
                     "." STRINGIFY_VALUE( VERSION_MINOR ) \
                     "." STRINGIFY_VALUE( VERSION_PATCH ) \
                     "." STRINGIFY_VALUE( VERSION_BUILD )

/// The full version number as a wide string
#define FULL_VERSION_W L"" FULL_VERSION
