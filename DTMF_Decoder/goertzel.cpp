///////////////////////////////////////////////////////////////////////////////
//          University of Hawaii, College of Engineering
//          DTMF_Decoder22X!Vqrpp1kz9C!ma3mCkbd - EE 469 - Fall 2022
//
/// A Windows Desktop C program that decodes DTMF tones
/// 
/// @file goertzel.cpp
/// @version 1.0
///
/// @author Mark Nelson <marknels@hawaii.edu>
/// @date   10_Oct_2022
///////////////////////////////////////////////////////////////////////////////

#include "framework.h"
#include "DTMF_Decoder.h" 
#include "mvcModel.h"
#include <stdint.h>
#define _USE_MATH_DEFINES // for C++  
#include <math.h>


float goertzel_magnitude( const uint16_t iNumSamples, const uint16_t targetFrequency, const uint16_t samplingRate, const BYTE pcmData[] ) {

   float scalingFactor = iNumSamples / 2.0;

   float floatnumSamples = (float) iNumSamples;

   int k = (int) ( 0.5 + ( ( floatnumSamples * targetFrequency ) / samplingRate ) );

   float omega = ( 2.0 * M_PI * k ) / floatnumSamples;

   float sine = sin( omega );
   float cosine = cos( omega );
   float coeff = 2.0 * ( cosine );

   int q0 = 0;
   int q1 = 0;
   int q2 = 0;

   for ( uint16_t i = 0 ; i < iNumSamples ; i++ ) {
      q0 = coeff * q1 - q2 + (float)pcmData[ i ];
      q2 = q1;
      q1 = q0;
   }

   float real = ( q1 * cosine - q2 ) / scalingFactor;
   float imag = ( q1 * sine ) / scalingFactor;

   return sqrtf( real * real + imag * imag );
}


BOOL compute_dtmf_tones_with_goertzel() {
   float m = 0;
   float threshold = 10;

   for ( UINT8 i = 0 ; i < NUMBER_OF_DTMF_TONES ; i++ ) {
      m = goertzel_magnitude( SIZE_OF_QUEUE, dtmfTones[i].frequency, 8000, pcmQueue);
      // TODO: Replace 8000 with a global

      if ( m >= threshold ) {
         editToneDetectedStatus( i, true );
      } else {
         editToneDetectedStatus( i, false );
      }
   }

   return TRUE;
}
