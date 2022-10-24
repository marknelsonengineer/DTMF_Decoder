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
#include "audio.h"  // for getSamplesPerSecond()


float goertzel_magnitude( int numSamples, float TARGET_FREQUENCY, int SAMPLING_RATE ) {
   int     k, i;
   float   floatnumSamples;
   float   omega, sine, cosine, coeff, q0, q1, q2, magnitude, real, imag;

   float   scalingFactor = numSamples / 2.0;

   // float data;

   floatnumSamples = (float) numSamples;
   k = (int) ( 0.5 + ( ( floatnumSamples * TARGET_FREQUENCY ) / (float) SAMPLING_RATE ) );
   omega = ( 2.0 * M_PI * k ) / floatnumSamples;
   sine = sin( omega );
   cosine = cos( omega );
   coeff = 2.0 * cosine;
   q0 = 0;
   q1 = 0;
   q2 = 0;

   pcmResetReadQueue();

   for ( i = 0; i < numSamples; i++ ) {
      q0 = coeff * q1 - q2 + ( (float) pcmReadQueue() );
      q2 = q1;
      q1 = q0;
   }

   // calculate the real and imaginary results
   // scaling appropriately
   real = ( q1 * cosine - q2 ) / scalingFactor;
   imag = ( q1 * sine ) / scalingFactor;

   magnitude = sqrtf( real * real + imag * imag );
   //phase = atan(imag/real)
   return magnitude;
}


BOOL compute_dtmf_tones_with_goertzel() {
   float threshold = 10;

   for ( UINT8 i = 0 ; i < NUMBER_OF_DTMF_TONES ; i++ ) {
      dtmfTones[i].goertzelMagnitude = goertzel_magnitude(pcmGetQueueSize(), dtmfTones[i].frequency, getSamplesPerSecond() );

      if ( dtmfTones[ i ].goertzelMagnitude >= threshold ) {
         editToneDetectedStatus( i, true );
      } else {
         editToneDetectedStatus( i, false );
      }
   }
   
   return TRUE;
}
