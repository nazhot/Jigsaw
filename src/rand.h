#ifndef RAND_H
#define RAND_H

#include <stdio.h>
#include <stdlib.h>

size_t rand_index( size_t size );
int rand_intBetween( int lowerBound, int upperBound );
float rand_float();
float rand_floatBetween( float lowerBound, float upperBound );
void rand_shuffle( void *array, size_t numElements, ssize_t elementSize );


#endif
