#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "puzzle.h"
#include "pieces.h"

int main( int argc, char *argv[] ) {

/*
    srand( time( NULL ) );
    Puzzle *puzzle = puzzle_create( 6 );
    for ( uint i = 0; i < 500; ++i ) {
        uint numIndexConnections = 0;
        uint numSideConnections = 0;
        uint count = puzzle_findValidSolutions( puzzle, &numIndexConnections, &numSideConnections );
        if ( count == 1 ) {
            puzzle_shuffle( puzzle );
            continue;
        }
        for ( uint j = 0; j < 40; ++j ) {
            if ( j ) {
                printf( ", " );
            }
            printf( "%i", puzzle->connections[j] );
        }
        printf( "\n" );
        puzzle_shuffle( puzzle );
    }
    puzzle_free( puzzle );
*/
    puzzle_findMostUniqueSolution( 20, 10000, 100000, 40, 50, 1, 10 );

/*
    for ( uint unique = 6; unique < 21; ++unique ) {
        Puzzle* puzzle = puzzle_create( unique );
        uint counts[1050] = {0};
        uint maxSum = 0;
        for ( uint i = 0; i < 1000000; ++i ) {
            //while ( true ) {
            uint maxUniqueIndexes = 0;
            uint maxUniqueSides = 0;
            uint count = puzzle_findValidSolutions( puzzle, &maxUniqueIndexes,
                                                    &maxUniqueSides );
            if ( count == 2 && maxUniqueIndexes + maxUniqueSides > maxSum ) {
                maxSum = maxUniqueIndexes + maxUniqueSides;
            }
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
        printf( "Max Unique Sum: %u\n", maxSum );
        printf( "----------------\n" );
        puzzle_free( puzzle );
    }
*/

    return 0;
}
