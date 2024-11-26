#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "puzzle.h"
#include "pieces.h"

int main( int argc, char *argv[] ) {

    srand( time( NULL ) );
    Puzzle* puzzle = puzzle_create( 6 );
    uint count;
    uint numPuzzles = 1;

    for ( uint i = 0; i < 1000000; ++i ) {
        puzzle_findValidSolutions( puzzle );
        puzzle_shuffle( puzzle );
    }


    printf( "Puzzles: %u\n", numPuzzles );

    return 0;
}
