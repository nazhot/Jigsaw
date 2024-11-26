#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "puzzle.h"
#include "pieces.h"

int main( int argc, char *argv[] ) {

    srand( time( NULL ) );
    Puzzle* puzzle = puzzle_create( 6 );

    //for ( uint i = 0; i < 1000000; ++i ) {
    while ( true ) {
        puzzle_findValidSolutions( puzzle );
        puzzle_shuffle( puzzle );
    }

    return 0;
}
