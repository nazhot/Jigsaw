#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "benchmark.h"
#include "puzzle.h"
#include "rand.h"

const uint numEdgeConnections = 16;
const uint numTotalConnections = 40;
const uint numCenterConnections = numTotalConnections - numEdgeConnections;


static void generateCenter( const uint numUniqueConnections, const uint numUniqueEdgeConnections,
                            const uint centerConnections[numCenterConnections],
                            uint centerContains[numUniqueConnections][numCenterConnections], 
                            uint centerContainsCounts[numUniqueConnections] ) {
    static const int centerIndexes[24][2] = { { 6, -1 }, { 6, 7 }, { 7, 8 }, { 8, -1 },
                                              { 11, -1 }, { 11, 12 }, { 12, 13 }, { 13, -1 },
                                              { 16, -1 }, { 16, 17 }, { 17, 18 }, { 18, -1 },
                                              { 6, -1 }, { 6, 11 }, { 11, 16 }, { 16, -1 },
                                              { 7, -1 }, { 7, 12 }, { 12, 17 }, { 17, -1 },
                                              { 8, -1 }, { 8, 13 }, { 13, 18 }, { 18, -1 } };

    memset( centerContainsCounts, 0, sizeof( uint ) * numUniqueConnections );

    for ( uint i = 0; i < numCenterConnections; ++i ) {
        const uint connection = centerConnections[i];
        for ( uint j = 0; j < 2; ++j ) {
            const int index = centerIndexes[i][j];
            if ( index == -1 ) {
                break;
            }
            centerContains[connection][centerContainsCounts[connection]] = centerIndexes[i][j];
            ++centerContainsCounts[connection];
        }
    }
}

static void generateEdge( const uint numUniqueConnections, const uint numUniqueEdgeConnections,
                          const uint edgeConnections[numEdgeConnections],
                          uint leftEdges[numUniqueEdgeConnections][numEdgeConnections],
                          uint leftEdgesCounts[numUniqueEdgeConnections],
                          uint rightEdges[numUniqueEdgeConnections][numEdgeConnections],
                          uint rightEdgesCounts[numUniqueEdgeConnections] ) {
    static const uint leftEdgeIndexes[16] = { 1, 2, 3, 4, 9, 14, 19, 24, 23, 22, 21, 20, 15, 10, 5, 0 };
    static const uint rightEdgeIndexes[16] = { 0, 1, 2, 3, 4, 9, 14, 19, 24, 23, 22, 21, 20, 15, 10, 5 };
    
    memset( leftEdgesCounts, 0, sizeof( uint ) * numUniqueEdgeConnections );
    memset( rightEdgesCounts, 0, sizeof( uint ) * numUniqueEdgeConnections );

    for ( uint i = 0; i < numEdgeConnections; ++i ) {
        const uint connection = edgeConnections[i];
        leftEdges[connection][leftEdgesCounts[connection]] = leftEdgeIndexes[i];
        ++leftEdgesCounts[connection];

        rightEdges[connection][rightEdgesCounts[connection]] = rightEdgeIndexes[i];
        ++rightEdgesCounts[connection];
    }
}

void generateSwappablePuzzle( const uint numUniqueConnections ) {
    const uint maxTotal = numTotalConnections - ( numUniqueConnections * 2 ) + 2;
    const uint maxUniqueConnectionsInEdge = numUniqueConnections > ( numEdgeConnections / 2 ) ? numEdgeConnections / 2 : numUniqueConnections;
    const uint minUniqueConnectionsInEdge = numUniqueConnections <= ( numCenterConnections / 2 ) ? 
                                            1 : numUniqueConnections - ( numCenterConnections / 2 );

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

    uint leftEdges[numUniqueEdgeConnections][numEdgeConnections];
    uint leftEdgesCounts[numUniqueEdgeConnections];

    uint rightEdges[numUniqueEdgeConnections][numEdgeConnections];
    uint rightEdgesCounts[numUniqueEdgeConnections];

    generateEdge( numUniqueConnections, numUniqueEdgeConnections, edgeConnections,
                  leftEdges, leftEdgesCounts, rightEdges, rightEdgesCounts );

    uint centerContains[numUniqueConnections][numCenterConnections];
    uint centerContainsCounts[numUniqueConnections];

    generateCenter( numUniqueConnections, numUniqueEdgeConnections,
                    centerConnections, centerContains, centerContainsCounts );


    uint validEdges[1320][3];
    uint numValidEdges = 0;

    static const uint cornerIndexes[4] = { 0, 4, 20, 24 };
    static const uint edgeIndexes[12] = { 1, 2, 3, 5, 10, 15, 9, 14, 19, 21, 22, 23 };

    //Right/Left refer to Right/Left of edge pieces
/*
    for ( uint i = 0; i < 12; ++i ) { //left

        const Piece first = puzzle->pieces[edgeIndexes[i]];
        const char firstLeft = piece_getSide( first, LEFT );

        if ( !charArrayContains( validLefts, 4, firstLeft ) ) {
            continue;
        }

        const char firstRight = piece_getSide( first, RIGHT );

        for ( uint j = 0; j < 12; ++j ) {
            if ( j == i ) {
                continue;
            }
            const Piece second = puzzle->pieces[edgeIndexes[j]];
            const char secondLeft = piece_getSide( second, LEFT );

            if ( !piece_piecesConnect( secondLeft, firstRight ) ) {
                continue;
            }

            const char secondRight = piece_getSide( second, RIGHT );

            for ( uint k = 0; k < 12; ++k ) {
                if ( i == k || j == k ) {
                    continue;
                }
                const Piece third = puzzle->pieces[edgeIndexes[k]];
                const char thirdLeft = piece_getSide( third, LEFT );

                if ( !piece_piecesConnect( secondRight, thirdLeft ) ){ 
                    continue;
                }

                const char thirdRight = piece_getSide( third, RIGHT );

                if ( !charArrayContains( validRights, 4, thirdRight ) ) {
                    continue; 
                }

                TripleIndex tempTripleIndex;

                tempTripleIndex.indexes[0] = first.index;
                tempTripleIndex.indexes[1] = second.index;
                tempTripleIndex.indexes[2] = third.index;
                
                da_addElement( validEdges, &tempTripleIndex );
            }
        }
    }
*/
     
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
