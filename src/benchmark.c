#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "benchmark.h"
#include "puzzle.h"
#include "rand.h"

void generateEdge( const uint numUniqueConnections ) {
    const uint numEdgeConnections = 16;
    const uint numTotalConnections = 40;
    const uint maxTotal = numTotalConnections - ( numUniqueConnections * 2 ) + 2;
    const uint maxUniqueConnectionsInEdge = numUniqueConnections > numEdgeConnections / 2 ? numEdgeConnections / 2 : numUniqueConnections;
    uint minUniqueConnectionsInEdge;
    if ( maxTotal >= numEdgeConnections ) {
        minUniqueConnectionsInEdge = 1;
    } else {
        minUniqueConnectionsInEdge = numEdgeConnections / maxTotal;
        if ( numEdgeConnections % maxTotal ) {
            ++minUniqueConnectionsInEdge;
        }
    }
    printf( "%u, %u, %u\n", numUniqueConnections, maxTotal, minUniqueConnectionsInEdge );

    const uint numUniqueEdgeConnections = rand_intBetween( minUniqueConnectionsInEdge,
                                                           maxUniqueConnectionsInEdge + 1 );
    const uint numRemainingEdges = numEdgeConnections - ( numUniqueEdgeConnections * 2 );
    uint edgeCounts[numUniqueEdgeConnections];
    for ( uint i = 0; i < numUniqueEdgeConnections; ++i ) {
        edgeCounts[i] = 2;
    }

    for ( uint i = 0; i < numRemainingEdges; ++i ) {
        const uint index = rand_index( numUniqueEdgeConnections );
        ++edgeCounts[index];
    }

    uint edgeConnections[numEdgeConnections];
    uint count = 0;
    for ( uint i = 0; i < numUniqueEdgeConnections; ++i ) {
        for ( uint j  = 0; j < edgeCounts[i]; ++j ) {
            edgeConnections[count] = i;
            ++count;
        }
    }

    rand_shuffle( edgeConnections, numEdgeConnections, sizeof( uint ) );
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
