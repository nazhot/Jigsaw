#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "puzzle.h"
#include "pieces.h"

int main( int argc, char *argv[] ) {

    srand( time( NULL ) );

    Puzzle *puzzle = puzzle_create( 6 );
    for ( uint i = 0; i < 5; ++i ) {
        puzzle_findValidSolutions( puzzle );
        puzzle_shuffle( puzzle );
    }
    puzzle_free( puzzle );


/*
    for ( uint unique = 6; unique < 21; ++unique ) {
        Puzzle* puzzle = puzzle_create( unique );
        uint counts[1050] = {0};
        for ( uint i = 0; i < 1000000; ++i ) {
            //while ( true ) {
            uint count = puzzle_findValidSolutions( puzzle );
            ++counts[count];
            puzzle_shuffle( puzzle );
        }

        printf( "-------%u--------\n", unique );
        for ( uint i = 0; i < 200; ++i ) {
            if ( !counts[i] ) {
                continue;
            }
            printf( "%u: %u\n", i, counts[i] );
        }
        printf( "----------------\n" );
        puzzle_free( puzzle );
    }
*/

    return 0;
}
