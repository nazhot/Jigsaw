#include "puzzle.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rand.h"
#include "pieces.h"


typedef struct TripleIndex {
    char indexes[3];
    char rotations[3];
} TripleIndex;

typedef struct EdgeSolution {
    char cornerIndexes[4];
    char topEdgeIndexes[3]; //left to right
    char leftEdgeIndexes[3]; //top to bottom
    char rightEdgeIndexes[3]; //top to bottom
    char bottomEdgeIndexes[3]; //left to right
} EdgeSolution;

typedef struct CenterSolution {
    char indexes[3][3];
    char rotations[3][3];
} CenterSolution;

typedef struct PuzzleSolution {
    CenterSolution* centers;
    EdgeSolution* edges;
} PuzzleSolution;


static void puzzle_printEdgeSolution( const EdgeSolution* const edgeSolution ) {
    printf( "%02i %02i %02i %02i %02i\n", edgeSolution->cornerIndexes[0],
                                          edgeSolution->topEdgeIndexes[0],
                                          edgeSolution->topEdgeIndexes[1],
                                          edgeSolution->topEdgeIndexes[2],
                                          edgeSolution->cornerIndexes[1] );
    for ( uint i = 0; i < 3; ++i ) {
        printf( "%02i          %02i\n", edgeSolution->leftEdgeIndexes[i],
                                            edgeSolution->rightEdgeIndexes[i] );
    }
    printf( "%02i %02i %02i %02i %02i\n", edgeSolution->cornerIndexes[3],
                                     edgeSolution->bottomEdgeIndexes[0],
                                     edgeSolution->bottomEdgeIndexes[1],
                                     edgeSolution->bottomEdgeIndexes[2],
                                     edgeSolution->cornerIndexes[2] );
}

static void puzzle_printSolution( const EdgeSolution* const edgeSolution,
                                  const CenterSolution* const centerSolution ) {
    printf( "%02i %02i %02i %02i %02i\n", edgeSolution->cornerIndexes[0],
                                          edgeSolution->topEdgeIndexes[0],
                                          edgeSolution->topEdgeIndexes[1],
                                          edgeSolution->topEdgeIndexes[2],
                                          edgeSolution->cornerIndexes[1] );
    for ( uint i = 0; i < 3; ++i ) {
        printf( "%02i %02i %02i %02i %02i\n", edgeSolution->leftEdgeIndexes[i],
                                              centerSolution->indexes[i][0],
                                              centerSolution->indexes[i][1],
                                              centerSolution->indexes[i][2],
                                              edgeSolution->rightEdgeIndexes[i] );
    }
    printf( "%02i %02i %02i %02i %02i\n", edgeSolution->cornerIndexes[3],
                                          edgeSolution->bottomEdgeIndexes[0],
                                          edgeSolution->bottomEdgeIndexes[1],
                                          edgeSolution->bottomEdgeIndexes[2],
                                          edgeSolution->cornerIndexes[2] );
    printf( "Rotations:\n" );
    for ( uint i = 0; i < 3; ++i ) {
        printf( "%i %i %i\n", centerSolution->rotations[i][0], centerSolution->rotations[i][1],
                              centerSolution->rotations[i][2] );
    }
}

static bool charArrayContains( const char* const array, const uint arraySize,
                                          const char value ) {
    for ( uint i = 0; i < arraySize; ++i ) {
        if ( array[i] == value ) {
            return true;
        }
    }
    return false;
}

static TripleIndex* puzzle_calculateValidEdges( const Puzzle* const puzzle,
                                                uint* const numSolutions ) {
    //Right/Left refer to Right/Left of edge pieces
    char validLefts[4] = {0};
    char validRights[4] = {0};
    for ( uint i = 0; i < 4; ++i ) {
        const Piece* corner = puzzle->cornerPieces[i];
        validLefts[i] = -piece_getSide( *corner, RIGHT );
        validRights[i] = -piece_getSide( *corner, LEFT );
    }

    char validTriples[1320][3];
    uint numValidTriples = 0;
    for ( uint i = 0; i < 12; ++i ) { //left
        const Piece* first = puzzle->edgePieces[i];
        const char firstLeft = piece_getSide( *first, LEFT );

        if ( !charArrayContains( validLefts, 4, firstLeft ) ) {
            continue;
        }

        const char firstRight = piece_getSide( *first, RIGHT );

        for ( uint j = 0; j < 12; ++j ) {
            const Piece* second = puzzle->edgePieces[j];
            const char secondLeft = piece_getSide( *second, LEFT );

            if ( j == i || secondLeft + firstRight != 0 ) {
                continue;
            }

            const char secondRight = piece_getSide( *second, RIGHT );

            for ( uint k = 0; k < 12; ++k ) {
                const Piece* third = puzzle->edgePieces[k];
                const char thirdRight = piece_getSide( *third, RIGHT );

                if ( !charArrayContains( validRights, 4, thirdRight ) ) {
                    continue; 
                }

                const char thirdLeft = piece_getSide( *third, LEFT );

                if ( i == k || j == k || secondRight + thirdLeft != 0 ) {
                    continue;
                }

                validTriples[numValidTriples][0] = first->index;
                validTriples[numValidTriples][1] = second->index;
                validTriples[numValidTriples][2] = third->index;
                ++numValidTriples;
            }
        }
    }
    *numSolutions = numValidTriples;
    TripleIndex* solutions = malloc( sizeof( TripleIndex ) * numValidTriples );
    if ( !solutions ) {
        fprintf( stderr, "Could not allocate TripleSol\n" );
        exit( 1 );
    }
    for ( uint i = 0; i < numValidTriples; ++i ) {
        for ( uint j = 0; j < 3; ++j ) {
            solutions[i].indexes[j] = validTriples[i][j];
        }
    }

    return solutions;
}

static void puzzle_recEdgeSolve( const Puzzle* const puzzle, char edgeIndexes[4],
                          const char* const currentArrangement,
                          const TripleIndex* const edgeTriples, const uint numEdgeTriples,
                          const uint currentEdge, EdgeSolution* const edgeSolutions,
                          uint* const numEdgeSolutions ) {
    const char leftCorner = piece_getSide( puzzle->pieces[( int ) currentArrangement[( int ) currentEdge]], RIGHT );
    const char rightCorner = piece_getSide( puzzle->pieces[( int ) currentArrangement[( int  ) currentEdge + 1]], LEFT );

    for ( int i = 0; i < numEdgeTriples; ++i ) {
        if ( charArrayContains( edgeIndexes, currentEdge, i ) ) {
         continue;   
        }

        bool valid = true;
        for ( uint j = 0; j < currentEdge; ++j ) {
            for ( uint k = 0; k < 3; ++k ) {
                if ( charArrayContains( edgeTriples[( int ) edgeIndexes[j]].indexes, 3, edgeTriples[i].indexes[k] ) ) {
                    valid = false; 
                    break;
                }
            }
            if ( !valid ) {
                break;
            }
        }
        if ( !valid ) {
            continue;
        }
        const char leftEdge = piece_getSide( puzzle->pieces[( int  ) edgeTriples[i].indexes[0]], LEFT );
        if ( leftEdge + leftCorner != 0  ){
            continue;
        }
        const char rightEdge = piece_getSide( puzzle->pieces[( int ) edgeTriples[i].indexes[2]], RIGHT );
        if ( rightEdge + rightCorner != 0 ) {
            continue;
        }
        edgeIndexes[currentEdge] = i;
        if ( currentEdge == 3 ) {
            for ( uint i = 0; i < 4; ++i ) {
                edgeSolutions[*numEdgeSolutions].cornerIndexes[i] = currentArrangement[i]; 
                if ( i < 3 ) {
                    edgeSolutions[*numEdgeSolutions].topEdgeIndexes[i] = edgeTriples[ ( int ) edgeIndexes[0]].indexes[i];
                    edgeSolutions[*numEdgeSolutions].rightEdgeIndexes[i] = edgeTriples[ ( int ) edgeIndexes[1]].indexes[i];   
                    edgeSolutions[*numEdgeSolutions].bottomEdgeIndexes[i] = edgeTriples[ ( int ) edgeIndexes[2]].indexes[2 - i];   
                    edgeSolutions[*numEdgeSolutions].leftEdgeIndexes[i] = edgeTriples[ ( int ) edgeIndexes[3]].indexes[2 - i];   
                }
            }
            ++*numEdgeSolutions;
        } else {
            puzzle_recEdgeSolve( puzzle, edgeIndexes, currentArrangement, 
                                 edgeTriples, numEdgeTriples, currentEdge + 1,
                                 edgeSolutions, numEdgeSolutions );
        }
    }
}

static TripleIndex* puzzle_calculateValidCenterRows( const Puzzle* const puzzle,
                                                     const EdgeSolution* edgeSolution,
                                                     uint* const numSolutions ) { 

    //Right/Left refer to Right/Left of edge pieces
    char validLefts[3] = {0};
    char validRights[3] = {0};
    for ( uint i = 0; i < 3; ++i ) {
        const Piece* leftEdge = &puzzle->pieces[( int ) edgeSolution->leftEdgeIndexes[i]];
        const Piece* rightEdge = &puzzle->pieces[( int ) edgeSolution->rightEdgeIndexes[i]];
        validLefts[i]= -piece_getSide( *leftEdge, BOTTOM );
        validRights[i]= -piece_getSide( *rightEdge, BOTTOM );
    }

    TripleIndex validTriples[13200];
    uint numValidTriples = 0;

    for ( uint i = 0; i < 36; ++i ) {
        const uint firstIndex = i / 4; 
        const uint firstRotation = i % 4;
        const Piece* firstPiece = puzzle->centerPieces[firstIndex];
        const char firstLeft = piece_getSideWithRotation( *firstPiece, LEFT, firstRotation );
        if ( !charArrayContains( validLefts, 3, firstLeft ) ) {
            continue;
        }
        const char firstRight = piece_getSideWithRotation( *firstPiece, RIGHT, firstRotation );
        for ( uint j = 0; j < 36; ++j ) {
            const uint secondIndex = j / 4; 
            if ( firstIndex == secondIndex ) {
                continue;
            }
            const uint secondRotation = j % 4;
            const Piece* secondPiece = puzzle->centerPieces[secondIndex];
            const char secondLeft = piece_getSideWithRotation( *secondPiece, LEFT, secondRotation );
            if ( firstRight + secondLeft != 0 ) {
                continue;
            }
            const char secondRight = piece_getSideWithRotation( *secondPiece, RIGHT, secondRotation );
            for ( uint k = 0; k < 36; ++k ) {
                const uint thirdIndex = k / 4; 
                if ( thirdIndex == secondIndex || thirdIndex == firstIndex ) {
                    continue;
                }
                const uint thirdRotation = k % 4;
                const Piece* thirdPiece = puzzle->centerPieces[thirdIndex];
                const char thirdLeft = piece_getSideWithRotation( *thirdPiece, LEFT, thirdRotation );
                if ( secondRight + thirdLeft != 0 ) {
                    continue;
                }
                const char thirdRight = piece_getSideWithRotation( *thirdPiece, RIGHT, thirdRotation );
                if ( !charArrayContains( validRights, 3, thirdRight ) ) {
                    continue;
                }

                validTriples[numValidTriples].indexes[0] = firstPiece->index;
                validTriples[numValidTriples].indexes[1] = secondPiece->index;
                validTriples[numValidTriples].indexes[2] = thirdPiece->index;
                validTriples[numValidTriples].rotations[0] = firstRotation;
                validTriples[numValidTriples].rotations[1] = secondRotation;
                validTriples[numValidTriples].rotations[2] = thirdRotation;
                ++numValidTriples;
                if ( numValidTriples >= 13200 ) {
                    printf( "Center error\n" );
                }
            }
        }
    }

    *numSolutions = numValidTriples;
    TripleIndex* solutions = malloc( sizeof( TripleIndex ) * numValidTriples );
    if ( !solutions ) {
        fprintf( stderr, "Could not allocate EdgeSolution\n" );
        exit( 1 );
    }
    for ( uint i = 0; i < numValidTriples; ++i ) {
        for ( uint j = 0; j < 3; ++j ) {
            solutions[i].indexes[j] = validTriples[i].indexes[j];
            solutions[i].rotations[j] = validTriples[i].rotations[j];
        }
    }

    return solutions;
}

void puzzle_recCenterSolve( const Puzzle* const puzzle, char centerIndexes[3],
                          const EdgeSolution* const edgeSolution,
                          const TripleIndex* const centerTriples, const uint numCenterTriples,
                          const uint currentRow, CenterSolution* const centerSolutions,
                          uint *numCenterSolutions ) {
    if ( currentRow == 3 ) {
        for ( uint i = 0; i < 3; ++i ) {
            TripleIndex row = centerTriples[( int ) centerIndexes[i]];
            for ( uint j = 0; j < 3; ++j ) {
                centerSolutions[*numCenterSolutions].indexes[i][j] = row.indexes[j];
                centerSolutions[*numCenterSolutions].rotations[i][j] = row.rotations[j];
            }
        }
        ++*numCenterSolutions;
        return;
    }

    char leftEdge = piece_getSide( puzzle->pieces[( int ) edgeSolution->leftEdgeIndexes[currentRow]], BOTTOM );
    char rightEdge = piece_getSide( puzzle->pieces[( int ) edgeSolution->rightEdgeIndexes[currentRow]], BOTTOM );

    for ( int i = 0; i < numCenterTriples; ++i ) {
        if ( charArrayContains( centerIndexes, currentRow, i ) ) {
            continue;   
        }

        TripleIndex row = centerTriples[i];

        bool valid = true;
        for ( uint j = 0; j < currentRow; ++j ) {
            TripleIndex checkRow = centerTriples[( int ) centerIndexes[j]];
            for ( uint k = 0; k < 3; ++k ) {
                if ( charArrayContains( checkRow.indexes, 3, row.indexes[k] ) ) {
                    valid = false;
                    break;
                }
            }
            if ( !valid ) {
                break;
            }
        }
        if ( !valid ) {
            continue;
        }
        char leftCenter = piece_getSide( puzzle->pieces[( int ) row.indexes[0]], LEFT );
        if ( leftCenter + leftEdge != 0  ){
            continue;
        }
        char rightCenter = piece_getSide( puzzle->pieces[( int ) row.indexes[2]], RIGHT );
        if ( rightCenter + rightEdge != 0 ) {
            continue;
        }
        centerIndexes[currentRow] = i;
        puzzle_recCenterSolve( puzzle, centerIndexes, edgeSolution, 
                             centerTriples, numCenterTriples, currentRow + 1,
                             centerSolutions, numCenterSolutions );
    }
}

uint puzzle_findValidSolutions( const Puzzle* const puzzle ) {
    //only 6 valid arangements of corners (top left, top right, bottom right, bottom left)
    const static char cornerArrangements[6][5] = { {0, 4, 20, 24, 0}, {0, 4, 24, 20, 0},
                                                   {0, 20, 4, 24, 0}, {0, 20, 24, 4, 0},
                                                   {0, 24, 4, 20, 0}, {0, 24, 20, 4, 0} };
    //get the valid triplets of edges
    uint numValidEdges = 0;
    TripleIndex* validEdges = puzzle_calculateValidEdges( puzzle, &numValidEdges );
    if ( numValidEdges < 4 ) {
        printf( "Error in edge solver\n" );
    }

    //for all of the valid configurations, try all possible combinations of edges
    uint numEdgeSolutions = 0;
    EdgeSolution edgeSolutions[100] = {0};
    for ( uint i = 0; i < 6; ++i ) {
        char edges[4];
        uint temp = numEdgeSolutions;
        puzzle_recEdgeSolve( puzzle, edges, cornerArrangements[i], 
                             validEdges, numValidEdges, 0, edgeSolutions, &numEdgeSolutions );
    }

    uint numTotalConfigurations = 0;
    PuzzleSolution configurations[100];
    for ( uint i = 0; i < numEdgeSolutions; ++i ) {
        uint numValidCenters = 0;
        TripleIndex* validCenters = puzzle_calculateValidCenterRows( puzzle,
                                                                     &edgeSolutions[i],
                                                                     &numValidCenters );
        char centerIndexes[3];
        uint numCenterSolutions = 0;
        CenterSolution centerSolutions[100];
        puzzle_recCenterSolve( puzzle, centerIndexes, &edgeSolutions[i],
                               validCenters, numValidCenters, 0,
                               centerSolutions, &numCenterSolutions);
        numTotalConfigurations += numCenterSolutions;
        for  ( uint j = 0; j < numCenterSolutions; ++j ) {
            puzzle_printSolution( &edgeSolutions[i], &centerSolutions[j] );
        }
        free( validCenters );
    }

    free( validEdges );

    return numTotalConfigurations;
}

void puzzle_shuffle( Puzzle* const puzzle ) {
    uint numUniqueConnectors = puzzle->numUniqueConnectors;
    uint numEach = 40 / numUniqueConnectors;
    uint numLeftOver = 40 - numUniqueConnectors * numEach;
    for ( uint i = 0; i < numUniqueConnectors; ++i ) {
        for ( uint j = 0; j < numEach; ++j ) {
            puzzle->connections[i * numEach + j] = i + 1;            
            if ( rand_float() < 0.5 ) {
                puzzle->connections[i * numEach + j] *= -1;
            }
        }
    }

    for ( uint i = 0; i < numLeftOver; ++i ) {
        puzzle->connections[40  - numLeftOver + i] = rand_intBetween( 1, numUniqueConnectors + 1 );
        if ( rand_float() < 0.5 ) {
            puzzle->connections[40 - numLeftOver + i] *= -1;
        }
    }

    rand_shuffle( puzzle->connections, 40, sizeof( char ) );

    uint cornerIndex = 0;
    uint centerIndex = 0;
    uint edgeIndex = 0;

    for ( uint i = 0; i < 25; ++i ) {
        puzzle->rotations[i] = 0;
        if ( i == 0 ) {
            puzzle->pieces[0] = piece_create( CORNER, i );
            piece_setSide( &puzzle->pieces[0], RIGHT, puzzle->connections[0] );
            piece_setSide( &puzzle->pieces[0], LEFT, puzzle->connections[20] );
            puzzle->cornerPieces[cornerIndex++] = &puzzle->pieces[i];
            continue;
        } else if ( i == 4 ) {
            puzzle->pieces[4] = piece_create( CORNER, i );
            piece_setSide( &puzzle->pieces[4], RIGHT, puzzle->connections[24] );
            piece_setSide( &puzzle->pieces[4], LEFT, -puzzle->connections[15] );
            puzzle->cornerPieces[cornerIndex++] = &puzzle->pieces[i];
            continue;
        } else if ( i == 20 ) {
            puzzle->pieces[20] = piece_create( CORNER, i );
            piece_setSide( &puzzle->pieces[20], RIGHT, -puzzle->connections[35] );
            piece_setSide( &puzzle->pieces[20], LEFT, puzzle->connections[4] );
            puzzle->cornerPieces[cornerIndex++] = &puzzle->pieces[i];
            continue;
        } else if ( i == 24 ) {
            puzzle->pieces[24] = piece_create( CORNER, i );
            piece_setSide( &puzzle->pieces[24], RIGHT, -puzzle->connections[19] );
            piece_setSide( &puzzle->pieces[24], LEFT, -puzzle->connections[39] );
            puzzle->cornerPieces[cornerIndex++] = &puzzle->pieces[i];
            continue;
        } else if ( i < 4 ) { //top edge
            puzzle->pieces[i] = piece_create( EDGE, i );
            piece_setSide( &puzzle->pieces[i], RIGHT, puzzle->connections[i * 5] );
            piece_setSide( &puzzle->pieces[i], LEFT, -puzzle->connections[( i - 1 ) * 5] );
            piece_setSide( &puzzle->pieces[i], BOTTOM, puzzle->connections[i + 20] );
            puzzle->edgePieces[edgeIndex++] = &puzzle->pieces[i];
            continue;
        } else if ( i % 5 == 0 ) { //left edge
            puzzle->pieces[i] = piece_create( EDGE, i );
            piece_setSide( &puzzle->pieces[i], RIGHT, -puzzle->connections[i + 15] );
            piece_setSide( &puzzle->pieces[i], LEFT, puzzle->connections[i + 20] );
            piece_setSide( &puzzle->pieces[i], BOTTOM, puzzle->connections[i / 5] );
            puzzle->edgePieces[edgeIndex++] = &puzzle->pieces[i];
            continue;
        } else if ( ( i - 4) % 5 == 0 ) { //right edge
            puzzle->pieces[i] = piece_create( EDGE, i );
            piece_setSide( &puzzle->pieces[i], RIGHT, puzzle->connections[i + 20] );
            piece_setSide( &puzzle->pieces[i], LEFT, -puzzle->connections[i + 15] );
            piece_setSide( &puzzle->pieces[i], BOTTOM, -puzzle->connections[( i + 1 ) / 5 + 14] );
            puzzle->edgePieces[edgeIndex++] = &puzzle->pieces[i];
            continue;
        } else if ( i > 20 && i < 24 ) { //bottom edge
            puzzle->pieces[i] = piece_create( EDGE, i );
            piece_setSide( &puzzle->pieces[i], RIGHT, -puzzle->connections[( i - 21 ) * 5 + 4] );
            piece_setSide( &puzzle->pieces[i], LEFT, puzzle->connections[( i - 20 ) * 5 + 4] );
            piece_setSide( &puzzle->pieces[i], BOTTOM, -puzzle->connections[i + 15] );
            puzzle->edgePieces[edgeIndex++] = &puzzle->pieces[i];
            continue;
        }

        puzzle->pieces[i] = piece_create( CENTER, i );
        uint row = i / 5;
        uint col = i % 5;
        piece_setSide( &puzzle->pieces[i], TOP, -puzzle->connections[i + 15] );
        piece_setSide( &puzzle->pieces[i], RIGHT, puzzle->connections[5 * col + row] );
        piece_setSide( &puzzle->pieces[i], BOTTOM, puzzle->connections[i + 20] );
        piece_setSide( &puzzle->pieces[i], LEFT, -puzzle->connections[5 * ( col - 1 ) + row] );
        puzzle->centerPieces[centerIndex++] = &puzzle->pieces[i];
    }
}

Puzzle* puzzle_create( const uint numUniqueConnectors ) {
    Puzzle* puzzle = malloc( sizeof( Puzzle ) );
    if ( !puzzle ) {
        fprintf( stderr, "Could not allocate Puzzle\n" );
        exit( 1 );
    }
    puzzle->numUniqueConnectors = numUniqueConnectors;

    puzzle_shuffle( puzzle );

    return puzzle;
}

void puzzle_printLayout( const Puzzle* const puzzle ) {
    for ( uint i = 0; i < 25; ++i ) {
        if ( i % 5 == 0 ) {
            printf( "--------------------\n" );
        }
        piece_print( &puzzle->pieces[i] );
    }
}
