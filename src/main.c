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
        count = puzzle_solveEdges( puzzle );
        if ( !count ) {
            printf( "Error\n" );
            //puzzle_printLayout( puzzle );
            ++numPuzzles;
        }
        puzzle_shuffle( puzzle );
    }


    printf( "Puzzles: %u\n", numPuzzles );

    return 0;
}
