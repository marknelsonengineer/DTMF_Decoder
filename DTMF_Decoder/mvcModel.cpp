///////////////////////////////////////////////////////////////////////////////
//          University of Hawaii, College of Engineering
//          DTMF_Decoder - EE 469 - Fall 2022
//
/// A Windows Desktop C program that decodes DTMF tones
///
/// @file mvcModel.cpp
/// @version 1.0
///
/// @author Mark Nelson <marknels@hawaii.edu>
/// @date   10_Oct_2022
///////////////////////////////////////////////////////////////////////////////

#include "mvcModel.h"

dtmfTones_t dtmfTones[ 8 ] = {
   { 697.0, false},
   { 770.0, true},   /// @TODO Remove before flight
   { 852.0, false},
   { 941.0, false},
   {1209.0, false},
   {1336.0, true},
   {1477.0, false},
   {1633.0, false}
};
