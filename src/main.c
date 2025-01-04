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
    const uint numGenerations = 100;
    const uint numSurivors = 5;
    const uint numChildren = 800;
    const uint minMutations = 1;
    const uint maxMutations = 6;

    puzzle_findMostUniqueSolution( numUniqueConnections, generationSize, numGenerations,
                                   numSurivors, numChildren, minMutations, maxMutations );

    exit( 0 );

    Puzzle* puzzle = puzzle_create( 6 );
    puzzle_shuffleUntilUniqueEdge( puzzle );
    Puzzle* temp = malloc( sizeof( Puzzle ) );

    uint count = 0;
    uint bestSum = 0;
    uint bestSides = 0;
    uint bestIndexes = 0;
    while ( true ) {
        uint maxUniqueIndexes = 0;
        uint maxUniqueSides = 0;
        uint maxOtherSolutions = 100;
        uint numOtherSolutions = 0;
        PuzzleSolution solutions[maxOtherSolutions];
        puzzle_findValidSolutions( puzzle, solutions,
                                  &numOtherSolutions, maxOtherSolutions,
                                  &maxUniqueIndexes, &maxUniqueSides );
        if ( numOtherSolutions == 1 ) {
            uint sum = maxUniqueSides + maxUniqueIndexes;
            if ( sum > bestSum ) {
                bestSum = sum;
                printf( "Found puzzle with only 1 other solution!\n" );
                puzzle_printSolution( &solutions[0] );
                printf( "%i: %i sides + %i indexes\n", sum, maxUniqueSides, maxUniqueIndexes );
            }
            //for ( uint j = 0; j < 40; ++j ) {
            //    if ( j ) {
            //        printf( ", " );
            //    }
            //    printf( "%i", puzzle->connections[j] );
            //}
            //printf( "\n" );
            
        }
        puzzle_mutateCenter( temp, puzzle, 1, 6 );
        memcpy( puzzle, temp, sizeof( Puzzle ) );
        ++count;
        if ( count == 10000 ) {
            count = 0;
            printf( "Starting new generation\n" );
            puzzle_shuffleUntilUniqueEdge( puzzle );
        }
    }

/*
    Puzzle* puzzle = puzzle_create( 7 );
    for ( uint i = 0; i < 2000000; ++i ) {
        solve( puzzle ); 
        puzzle_shuffle( puzzle );
    }
*/

    return 0;
}
