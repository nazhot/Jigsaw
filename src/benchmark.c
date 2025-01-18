#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "benchmark.h"
#include "puzzle.h"
#include "rand.h"

void generateEdge( const uint numUniqueConnections ) {
    static const uint leftEdgeIndexes[16] = { 1, 2, 3, 4, 9, 14, 19, 24, 23, 22, 21, 20, 15, 10, 5, 0 };
    static const uint rightEdgeIndexes[16] = { 0, 1, 2, 3, 4, 9, 14, 19, 24, 23, 22, 21, 20, 15, 10, 5 };
    const uint numEdgeConnections = 16;
    const uint numTotalConnections = 40;
    const uint numCenterConnections = numTotalConnections - numEdgeConnections;
    const uint maxTotal = numTotalConnections - ( numUniqueConnections * 2 ) + 2;
    const uint maxUniqueConnectionsInEdge = numUniqueConnections > ( numEdgeConnections / 2 ) ? numEdgeConnections / 2 : numUniqueConnections;
    const uint minUniqueConnectionsInEdge = numUniqueConnections <= ( numCenterConnections / 2 ) ? 
                                            1 : numUniqueConnections - ( numCenterConnections / 2 );
    printf( "%u: %u, %u, %u\n", numUniqueConnections, maxTotal, minUniqueConnectionsInEdge, maxUniqueConnectionsInEdge );

    const uint numUniqueEdgeConnections = rand_intBetween( minUniqueConnectionsInEdge,
                                                           maxUniqueConnectionsInEdge + 1 );

    uint edgeConnections[numEdgeConnections];
    uint count = 0;
    for ( uint i = 0; i < numUniqueEdgeConnections; ++i ) {
        for ( uint j  = 0; j < 2; ++j ) {
            edgeConnections[count] = i;
            ++count;
        }
    }

    for ( uint i = count; i < numEdgeConnections; ++i ) {
        edgeConnections[i] = rand_index( numUniqueEdgeConnections );
    }

    rand_shuffle( edgeConnections, numEdgeConnections, sizeof( uint ) );

    uint leftEdges[numUniqueEdgeConnections][numEdgeConnections];
    uint leftEdgesCounts[numUniqueEdgeConnections];
    memset( leftEdgesCounts, 0, sizeof( uint ) * numUniqueEdgeConnections );

    uint rightEdges[numUniqueEdgeConnections][numEdgeConnections];
    uint rightEdgesCounts[numUniqueEdgeConnections];
    memset( rightEdgesCounts, 0, sizeof( uint ) * numUniqueEdgeConnections );

    for ( uint i = 0; i < numEdgeConnections; ++i ) {
        const uint connection = edgeConnections[i];
        leftEdges[connection][leftEdgesCounts[connection]] = leftEdgeIndexes[i];
        ++leftEdgesCounts[connection];

        rightEdges[connection][rightEdgesCounts[connection]] = rightEdgeIndexes[i];
        ++rightEdgesCounts[connection];
    }

    printf( "Edge Connections/Indexes Arrays:\n" );
    for ( uint i = 0; i < numUniqueEdgeConnections; ++i ) {
        printf( "%u (Left): ", i );
        for ( uint j = 0; j < leftEdgesCounts[i]; ++j ) {
            if ( j ) {
                printf( ", " );
            }
            printf( "%u", leftEdges[i][j] );
        }
        printf( "\n" );

        printf( "%u (Right): ", i );
        for ( uint j = 0; j < rightEdgesCounts[i]; ++j ) {
            if ( j ) {
                printf( ", " );
            }
            printf( "%u", rightEdges[i][j] );
        }
        printf( "\n" );
    }

    uint centerConnections[numCenterConnections];
    count = 0;
    for ( uint i = numUniqueEdgeConnections; i < numUniqueConnections; ++i ) {
        for ( uint j = 0; j < 2; ++j ) {
            centerConnections[count] = i;
            ++count;
        }
    }

    for ( uint i = count; i < numCenterConnections; ++i ) {
        centerConnections[i] = rand_index( numUniqueConnections );
    }

    rand_shuffle( centerConnections, numCenterConnections, sizeof( uint ) );

    printf( "Edge Connections:\n" );
    for ( uint i = 0; i < numEdgeConnections; ++i ) {
        if ( i ) {
            printf( ", " );
        }
        printf( "%u", edgeConnections[i] );
    }
    printf( "\n" );

    printf( "Center Connections:\n" );
    for ( uint i = 0; i < numCenterConnections; ++i ) {
        if ( i ) {
            printf( ", " );
        }
        printf( "%u", centerConnections[i] );
    }
    printf( "\n\n" );
}




void benchmark_puzzleSolve( const uint numPuzzles, const char* const description ) {
    srand( 0 );
    Puzzle* puzzle = puzzle_create( 7 );
    unsigned long total = 0;
    uint maxUniqueIndexes;
    uint maxUniqueSides;
    PuzzleSolution otherSolutions[50000];
    uint numOtherSolutions;
    const uint maxOtherSolutions = 50000;

    clock_t startTime = clock();

    for ( uint i = 0; i < numPuzzles; ++i ) {
        numOtherSolutions = 0;
        puzzle_findValidSolutions( puzzle, otherSolutions, &numOtherSolutions,
                                   maxOtherSolutions, &maxUniqueIndexes, &maxUniqueSides );
        total += numOtherSolutions;
        puzzle_shuffle( puzzle );
    }

    clock_t endTime = clock();
    clock_t difference = endTime - startTime;
    int milliSeconds = difference * 1000 / CLOCKS_PER_SEC;

    printf( "--------Results for %s--------\n", description );
    printf( "Num puzzles solved: %u\n", numPuzzles );
    printf( "Clock start: %li\n", startTime );
    printf( "Clock end: %li\n", endTime );
    printf( "Total clock time: %li\n", difference );
    printf( "Human time: %d seconds, %d milliseconds\n", milliSeconds / 1000, milliSeconds % 1000 );
    printf( "Sum of otherSolutions: %lu\n", total );

}
