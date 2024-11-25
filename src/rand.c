#include "rand.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

size_t rand_index( size_t size ) {
    if ((size - 1) == RAND_MAX) {
        return rand();
    } 
    int end = RAND_MAX / size;
    end *= size;

    int r;
    while ((r = rand()) >= end);
    return r % size;
}

int rand_intBetween( int lowerBound, int upperBound ) {
    if ( lowerBound == upperBound ) {
        return lowerBound;
    }
    
    if ( lowerBound > upperBound ) {
        int temp = lowerBound;
        lowerBound = upperBound;
        upperBound = temp;
    }
    int random = rand_index( upperBound - lowerBound );
    return lowerBound + random;
}

float rand_float() {
    return ( 1.0 * rand() ) / ( 1.0 * RAND_MAX );
}

float rand_floatBetween( float lowerBound, float upperBound ) {
    if ( lowerBound == upperBound ) {
        return lowerBound;
    }
    
    if ( lowerBound > upperBound ) {
        int temp = lowerBound;
        lowerBound = upperBound;
        upperBound = temp;
    }
    float random = rand_float();
    return random * ( upperBound - lowerBound ) + lowerBound;
    
}

void rand_shuffle( void *array, size_t numElements, ssize_t elementSize ) {
    if ( !array || numElements <= 0 || elementSize <= 0 ) {
        return;
    }

    int randIndex;
    void *temp = malloc( elementSize ); 
    for ( int i = numElements - 1; i >= 1; --i ) {
        randIndex = rand_index( i + 1 );
        memcpy( temp, &array[i * elementSize], elementSize );
        memcpy( &array[i * elementSize], &array[randIndex * elementSize], elementSize );
        memcpy( &array[randIndex * elementSize], temp, elementSize );
    }

    

}
