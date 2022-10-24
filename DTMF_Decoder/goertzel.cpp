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


int numSamples = 0;
int SAMPLING_RATE = 0;
float   floatnumSamples = 0;
float   scalingFactor = 0;

BOOL goertxzel_init( int numSamplesIn, int SAMPLING_RATE_IN ) {
   numSamples = numSamplesIn;
   SAMPLING_RATE = SAMPLING_RATE_IN;

   floatnumSamples = (float) numSamples;

   scalingFactor = numSamples / 2.0;

   int     k;

   float   omega, sine, cosine, coeff;


   for ( int i = 0 ; i < NUMBER_OF_DTMF_TONES ; i++ ) {
      k = (int) ( 0.5 + ( ( floatnumSamples * dtmfTones[ i ].frequency ) / (float) SAMPLING_RATE ) );
      omega = ( 2.0 * M_PI * k ) / floatnumSamples;
      sine = sin( omega );
      cosine = cos( omega );
      coeff = 2.0 * cosine;

      dtmfTones[ i ].sine = sine;
      dtmfTones[ i ].cosine = cosine;
      dtmfTones[ i ].coeff = coeff;
   }
 
   return TRUE;
}


float goertzel_magnitude( UINT8 index ) {
   // int     k;
   
   // float   omega, sine, cosine, coeff;
   // float q0;
   // float q1, q2;
   float magnitude, real, imag;

   // float data;

   //k = (int) ( 0.5 + ( ( floatnumSamples * dtmfTones[index].frequency ) / (float) SAMPLING_RATE ) );
   //omega = ( 2.0 * M_PI * k ) / floatnumSamples;
   //sine = sin( omega );
   //cosine = cos( omega );
   //coeff = 2.0 * cosine;
  //  q0 = 0;
   float q1 = 0;
   float q2 = 0;

   pcmResetReadQueue();

   for ( int i = 0; i < numSamples; i++ ) {
      float q0 = dtmfTones[ index ].coeff * q1 - q2 + ( (float) pcmReadQueue() );
      q2 = q1;
      q1 = q0;
   }

   // calculate the real and imaginary results
   // scaling appropriately
   real = ( q1 * dtmfTones[ index ].cosine - q2 ) / scalingFactor;
   imag = ( q1 * dtmfTones[ index ].sine ) / scalingFactor;

   magnitude = sqrtf( real * real + imag * imag );
   //phase = atan(imag/real)
   return magnitude;
}


BOOL compute_dtmf_tones_with_goertzel() {
   float threshold = 5;

   for ( UINT8 i = 0 ; i < NUMBER_OF_DTMF_TONES ; i++ ) {
      dtmfTones[i].goertzelMagnitude = goertzel_magnitude( i );

      if ( dtmfTones[ i ].goertzelMagnitude >= threshold ) {
         editToneDetectedStatus( i, true );
      } else {
         editToneDetectedStatus( i, false );
      }
   }
   
   return TRUE;
}
