#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "puzzle.h"
#include "pieces.h"

int main( int argc, char *argv[] ) {

    srand( time( NULL ) );

    const uint numUniqueConnections = 7;
    const uint generationSize = 5000;
    const uint numGenerations = 1000000;
    const uint numSurivors = 5;
    const uint numChildren = 800;
    const uint minMutations = 1;
    const uint maxMutations = 6;

    //puzzle_findMostUniqueSolution( numUniqueConnections, generationSize, numGenerations,
    //                               numSurivors, numChildren, minMutations, maxMutations );


    puzzle_findSolutionsUniqueEdges();

    exit( 0 );

    

/*
    Puzzle* puzzle = puzzle_create( 7 );
    for ( uint i = 0; i < 2000000; ++i ) {
        solve( puzzle ); 
        puzzle_shuffle( puzzle );
    }
*/

    return 0;
}
