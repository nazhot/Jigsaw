#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "puzzle.h"
#include "pieces.h"

int main( int argc, char *argv[] ) {

    srand( time( NULL ) );

    const uint numUniqueConnections = 7;
    const uint generationSize = 100000;
    const uint numGenerations = 100000;
    const uint numSurivors = 40;
    const uint numChildren = 2000;
    const uint minMutations = 1;
    const uint maxMutations = 6;

    puzzle_findMostUniqueSolution( numUniqueConnections, generationSize, numGenerations,
                                   numSurivors, numChildren, minMutations, maxMutations );

/*
    Puzzle* puzzle = puzzle_create( 7 );
    for ( uint i = 0; i < 2000000; ++i ) {
        solve( puzzle ); 
        puzzle_shuffle( puzzle );
    }
*/

    return 0;
}
