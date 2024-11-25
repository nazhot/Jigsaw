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
        Piece* corner = puzzle->cornerPieces[i];
        validLefts[i] = -piece_getSide( *corner, RIGHT );
        validRights[i] = -piece_getSide( *corner, LEFT );
    }

    char validTriples[1320][3];
    uint numValidTriples = 0;
    for ( uint i = 0; i < 12; ++i ) { //left
        Piece* first = puzzle->edgePieces[i];
        char firstLeft = piece_getSide( *first, LEFT );

        if ( !charArrayContains( validLefts, 4, firstLeft ) ) {
            continue;
        }

        char firstRight = piece_getSide( *first, RIGHT );

        for ( uint j = 0; j < 12; ++j ) {
            Piece* second = puzzle->edgePieces[j];
            char secondLeft = piece_getSide( *second, LEFT );

            if ( j == i || secondLeft + firstRight != 0 ) {
                continue;
            }

            char secondRight = piece_getSide( *second, RIGHT );

            for ( uint k = 0; k < 12; ++k ) {
                Piece* third = puzzle->edgePieces[k];
                char thirdRight = piece_getSide( *third, RIGHT );

                if ( !charArrayContains( validRights, 4, thirdRight ) ) {
                    continue; 
                }

                char thirdLeft = piece_getSide( *third, LEFT );

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

void puzzle_recEdgeSolve( const Puzzle* const puzzle, char edgeIndexes[4],
                          const char* const currentArrangement,
                          const TripleIndex* const edgeTriples, const uint numEdgeSolutions,
                          const uint currentEdge, EdgeSolution* const edgeSolutions,
                          uint *numPuzzleSolutions ) {
    if ( currentEdge == 4 ) {
        //corners
        for ( uint i = 0; i < 4; ++i ) {
           edgeSolutions->cornerIndexes[i] = currentArrangement[i]; 
        }

        for ( uint i = 0; i < 3; ++i ) {
            edgeSolutions->topEdgeIndexes[i] = edgeTriples[ ( int ) edgeIndexes[0]].indexes[i];
            edgeSolutions->rightEdgeIndexes[i] = edgeTriples[ ( int ) edgeIndexes[1]].indexes[i];   
            edgeSolutions->bottomEdgeIndexes[i] = edgeTriples[ ( int ) edgeIndexes[2]].indexes[2 - i];   
            edgeSolutions->leftEdgeIndexes[i] = edgeTriples[ ( int ) edgeIndexes[3]].indexes[2 - i];   
        }
        ++*numPuzzleSolutions;
        return;
    }
    char leftCorner = piece_getSide( *puzzle->cornerPieces[( int ) currentArrangement[( int ) currentEdge]], RIGHT );
    char rightCorner = piece_getSide( *puzzle->cornerPieces[( int ) currentArrangement[( int  ) currentEdge + 1]], LEFT );

    for ( uint i = 0; i < numEdgeSolutions; ++i ) {
        bool valid = true;
        for ( uint j = 0; j < currentEdge; ++j ) {
            if ( edgeIndexes[j] == i ) {
                valid = false;
                break;
            }
        }
        if ( !valid ) {
            continue;
        }

        char leftEdge = piece_getSide( puzzle->pieces[edgeTriples[i].indexes[0]], LEFT );
        if ( leftEdge + leftCorner != 0  ){
            continue;
        }
        char rightEdge = piece_getSide( puzzle->pieces[edgeTriples[i].indexes[2]], RIGHT );
        if ( rightEdge + rightCorner != 0 ) {
            continue;
        }
        edgeIndexes[currentEdge] = i;
        puzzle_recEdgeSolve( puzzle, edgeIndexes, currentArrangement, 
                             edgeTriples, numEdgeSolutions, currentEdge + 1,
                             edgeSolutions, numPuzzleSolutions );
    }
}


static void puzzle_shiftIndexBySolution( Puzzle* const puzzle,
                                         const PuzzleSolution* const solution ) {
    for ( uint i = 0; i < 25; ++i ) {
        int newIndex = solution->indexes[i];
        if ( newIndex == -1 ) {
            puzzle->pieceIndexes[i] = i;
        } else {
            puzzle->pieceIndexes[i] = solution->indexes[i];
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
        const Piece* leftEdge = &puzzle->pieces[edgeSolution->leftEdgeIndexes[i]];
        const Piece* rightEdge = &puzzle->pieces[edgeSolution->rightEdgeIndexes[i]];
        validLefts[i]= -piece_getSide( *leftEdge, BOTTOM );
        validRights[i]= -piece_getSide( *rightEdge, BOTTOM );
    }

    TripleIndex validTriples[13200];
    uint numValidTriples = 0;

    for ( uint i = 0; i < 36; ++i ) {
        uint firstIndex = i / 4; 
        uint firstRotation = i % 4;
        Piece* firstPiece = puzzle->centerPieces[firstIndex];
        char firstLeft = piece_getSideWithRotation( *firstPiece, LEFT, firstRotation );
        if ( !charArrayContains( validLefts, 3, firstLeft ) ) {
            continue;
        }
        char firstRight = piece_getSideWithRotation( *firstPiece, RIGHT, firstRotation );
        for ( uint j = 0; j < 36; ++j ) {
            uint secondIndex = j / 4; 
            if ( firstIndex == secondIndex ) {
                continue;
            }
            uint secondRotation = j % 4;
            Piece* secondPiece = puzzle->centerPieces[secondIndex];
            char secondLeft = piece_getSideWithRotation( *secondPiece, LEFT, secondRotation );
            if ( firstRight + secondLeft != 0 ) {
                continue;
            }
            char secondRight = piece_getSideWithRotation( *secondPiece, RIGHT, secondRotation );
            for ( uint k = 0; k < 36; ++k ) {
                uint thirdIndex = k / 4; 
                if ( thirdIndex == secondIndex || thirdIndex == firstIndex ) {
                    continue;
                }
                uint thirdRotation = k % 4;
                Piece* thirdPiece = puzzle->centerPieces[thirdIndex];
                char thirdLeft = piece_getSideWithRotation( *thirdPiece, LEFT, thirdRotation );
                if ( secondRight + thirdLeft != 0 ) {
                    continue;
                }
                char thirdRight = piece_getSideWithRotation( *thirdPiece, RIGHT, thirdRotation );
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



static void printPuzzleSolution( const PuzzleSolution* const puzzleSolution ) {
    for ( uint i = 0; i < 5; ++i ) {
        for ( uint j = 0; j < 5; ++j ) {
            if ( j ) {
                printf( ", " );
            }
            printf( "%i (%i)", puzzleSolution->indexes[i * 5 + j], puzzleSolution->rotations[i * 5 + j] );
        }
        printf( "\n" );
    }   
}

PuzzleSolution* puzzle_findValidSolutions( const Puzzle* const puzzle ) {
    //only 6 valid arangements of corners (top left, top right, bottom right, bottom left)
    const static char cornerArrangements[6][5] = { {0, 1, 2, 3, 0}, {0, 1, 3, 2, 0},
                                                   {0, 2, 1, 3, 0}, {0, 2, 3, 1, 0},
                                                   {0, 3, 1, 2, 0}, {0, 3, 2, 1, 0} };
    //get the valid triplets of edges
    uint numValidEdges = 0;
    TripleIndex* solutions = puzzle_calculateValidEdges( puzzle, &numValidEdges );
    if ( numValidEdges < 4 ) {
        printf( "Error in edge solver\n" );
    }

    //for all of the valid configurations, try all possible combinations of edges
    uint numEdgeSolutions = 0;
    EdgeSolution edgeSolutions[100] = {0};
    for ( uint i = 0; i < 6; ++i ) {
        char edges[4];
        puzzle_recEdgeSolve( puzzle, edges, cornerArrangements[i], 
                             solutions, numValidEdges, 0, edgeSolutions, &numEdgeSolutions );
    }

    for ( uint i = 0; i < numEdgeSolutions; ++i ) {
        uint numCenterSolutions = 0;
        TripleIndex rowSolutions[100] = {0};
        puzzle_calculateValidCenterRows( puzzle, &edgeSolutions[i], &numCenterSolutions );
        printf( "Num center rows: %u\n", numCenterSolutions );
    
    }


    return NULL;
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
        puzzle->pieceIndexes[i] = i;
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
