#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "puzzle.h"
#include "pieces.h"

int main( int argc, char *argv[] ) {

    srand( time( NULL ) );
    Puzzle* puzzle = puzzle_create( 18 );
    uint count;
    uint numPuzzles = 1;

    for ( uint i = 0; i < 50; ++i ) {
        printf( "----Puzzle %u----\n", i );
        puzzle_findValidSolutions( puzzle );
        printf( "-----------------\n\n" );
        puzzle_shuffle( puzzle );
    }


    printf( "Puzzles: %u\n", numPuzzles );

    return 0;
}
