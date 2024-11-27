#include "puzzle.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rand.h"
#include "pieces.h"


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

static void puzzle_calculateValidEdges( const Puzzle* const puzzle,
                                                TripleIndex* const validEdges,
                                                uint* const numValidEdges,
                                                const uint maxValidEdges ) {
    //Right/Left refer to Right/Left of edge pieces
    char validLefts[4] = {0};
    char validRights[4] = {0};
    for ( uint i = 0; i < 4; ++i ) {
        const Piece* corner = puzzle->cornerPieces[i];
        validLefts[i] = -piece_getSide( *corner, RIGHT );
        validRights[i] = -piece_getSide( *corner, LEFT );
    }

    *numValidEdges = 0;
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

                validEdges[*numValidEdges].indexes[0] = first->index;
                validEdges[*numValidEdges].indexes[1] = second->index;
                validEdges[*numValidEdges].indexes[2] = third->index;
                ++*numValidEdges;
                if ( *numValidEdges == maxValidEdges ) {
                    fprintf( stderr, "Too many valid edges\n" );
                    exit( 1 );
                }
            }
        }
    }
}

static void puzzle_recEdgeSolve( const Puzzle* const puzzle, char edgeIndexes[4],
                          const char* const currentArrangement,
                          const TripleIndex* const edgeTriples, const uint numEdgeTriples,
                          const uint currentEdge, EdgeSolution* const edgeSolutions,
                          uint* const numEdgeSolutions, const uint maxEdgeSolutions ) {
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
            if ( *numEdgeSolutions == maxEdgeSolutions ) {
                fprintf( stderr, "Too many edge solutions\n" );
                exit( 1 );
            }
        } else {
            puzzle_recEdgeSolve( puzzle, edgeIndexes, currentArrangement, 
                                 edgeTriples, numEdgeTriples, currentEdge + 1,
                                 edgeSolutions, numEdgeSolutions, maxEdgeSolutions );
        }
    }
}

static void puzzle_calculateValidCenterRows( const Puzzle* const puzzle,
                                                     const EdgeSolution* edgeSolution,
                                                     TripleIndex* const validCenterRows,
                                                     uint* const numValidCenterRows,
                                                     const uint maxValidCenterRows ) { 

    //Right/Left refer to Right/Left of edge pieces
    char validLefts[3] = {0};
    char validRights[3] = {0};
    for ( uint i = 0; i < 3; ++i ) {
        const Piece* leftEdge = &puzzle->pieces[( int ) edgeSolution->leftEdgeIndexes[i]];
        const Piece* rightEdge = &puzzle->pieces[( int ) edgeSolution->rightEdgeIndexes[i]];
        validLefts[i]= -piece_getSide( *leftEdge, BOTTOM );
        validRights[i]= -piece_getSide( *rightEdge, BOTTOM );
    }

    *numValidCenterRows = 0;

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

                validCenterRows[*numValidCenterRows].indexes[0] = firstPiece->index;
                validCenterRows[*numValidCenterRows].indexes[1] = secondPiece->index;
                validCenterRows[*numValidCenterRows].indexes[2] = thirdPiece->index;
                validCenterRows[*numValidCenterRows].rotations[0] = firstRotation;
                validCenterRows[*numValidCenterRows].rotations[1] = secondRotation;
                validCenterRows[*numValidCenterRows].rotations[2] = thirdRotation;
                ++*numValidCenterRows;
                if ( *numValidCenterRows == maxValidCenterRows ) {
                    printf( "Too many CenterRows error\n" );
                }
            }
        }
    }
}

void puzzle_recCenterSolve( const Puzzle* const puzzle, char centerIndexes[3],
                          const EdgeSolution* const edgeSolution,
                          const TripleIndex* const centerTriples, const uint numCenterTriples,
                          const uint currentRow, CenterSolution* const centerSolutions,
                          uint *numCenterSolutions, const uint maxCenterSolutions ) {
    if ( currentRow == 3 ) {
        for ( uint i = 0; i < 3; ++i ) {
            TripleIndex row = centerTriples[( int ) centerIndexes[i]];
            for ( uint j = 0; j < 3; ++j ) {
                centerSolutions[*numCenterSolutions].indexes[i][j] = row.indexes[j];
                centerSolutions[*numCenterSolutions].rotations[i][j] = row.rotations[j];
            }
        }
        ++*numCenterSolutions;
        if ( *numCenterSolutions == maxCenterSolutions ) {
            fprintf( stderr, "Too many center solutions\n" );
            exit( 1 );
        }
        return;
    }

    const char leftEdge = piece_getSide( puzzle->pieces[( int ) edgeSolution->leftEdgeIndexes[currentRow]], BOTTOM );
    const char rightEdge = piece_getSide( puzzle->pieces[( int ) edgeSolution->rightEdgeIndexes[currentRow]], BOTTOM );

    for ( int i = 0; i < numCenterTriples; ++i ) {
        if ( charArrayContains( centerIndexes, currentRow, i ) ) {
            continue;   
        }

        const TripleIndex row = centerTriples[i];

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
        const char leftCenter = piece_getSide( puzzle->pieces[( int ) row.indexes[0]], LEFT );
        if ( leftCenter + leftEdge != 0  ){
            continue;
        }
        const char rightCenter = piece_getSide( puzzle->pieces[( int ) row.indexes[2]], RIGHT );
        if ( rightCenter + rightEdge != 0 ) {
            continue;
        }

        if ( currentRow == 0 ) {
            for ( uint j = 0; j < 3; ++j ) {
                if ( piece_getSideWithRotation( puzzle->pieces[( int ) row.indexes[j]], TOP, row.rotations[j] ) +
                     piece_getSide( puzzle->pieces[( int ) edgeSolution->topEdgeIndexes[j]], BOTTOM ) != 0 ) {
                    valid = false;
                    break;
                }
            }
        } else if ( currentRow == 2 ) {
            for ( uint j = 0; j < 3; ++j ) {
                if ( piece_getSideWithRotation( puzzle->pieces[( int ) row.indexes[j]], BOTTOM, row.rotations[j] ) +
                     piece_getSide( puzzle->pieces[( int ) edgeSolution->bottomEdgeIndexes[j]], BOTTOM ) != 0 ) {
                    valid = false;
                    break;
                }
            }
        }
        if ( !valid ) {
            continue;
        }

        centerIndexes[currentRow] = i;
        puzzle_recCenterSolve( puzzle, centerIndexes, edgeSolution, 
                             centerTriples, numCenterTriples, currentRow + 1,
                             centerSolutions, numCenterSolutions, maxCenterSolutions );
    }
}

static uint puzzle_countChanges( const EdgeSolution* const edgeSolution,
                                  const CenterSolution* const centerSolution ) {
    const static char corners[] = { 0, 4, 24, 20 };
    uint count = 0;
    for ( uint i = 0; i < 3; ++i ) {
        if ( edgeSolution->topEdgeIndexes[i] != i + 1 ) {
            ++count;
        }
        if ( edgeSolution->leftEdgeIndexes[i] != ( i + 1 ) * 5 ) {
            ++count;
        }
        if ( edgeSolution->rightEdgeIndexes[i] != ( i + 1 ) * 5 + 4 ) {
            ++count;
        }
        if ( edgeSolution->bottomEdgeIndexes[i] != i + 21 ) {
            ++count;
        }
        for ( uint j = 0; j < 3; ++j ) {
            char index = ( i + 1 ) * 5 + 1 + j;
            if ( centerSolution->indexes[i][j] != index ) {
                ++count; 
            }
        }
    }

    for ( uint i = 0; i < 4; ++i ) {
        if ( edgeSolution->cornerIndexes[i] != corners[i] ) {
            ++count;
        }
    }
    return count;
}

static void puzzle_generateOriginalPiecePairs( PiecePair pairs[24][2]) {
    //vertical
    char connections[40][2];
    uint numConnections = 0;
    for ( uint row = 0; row < 5; ++row ) {
        for ( uint col = 0; col < 4; ++col ) {
            char index = row * 5 + col;
            connections[numConnections][0] = index;
            connections[numConnections][1] = index + 1;
            ++numConnections;
        }
    }

    for ( uint row = 0; row < 4; ++row ) {
        for ( uint col = 0; col < 5; ++col ) {
            char index = row * 5 + col;
            connections[numConnections][0] = index;
            connections[numConnections][1] = index + 5;
            ++numConnections;
        }
    }


    pairs[0][0] = ( PiecePair ) { .index = 1, .sides = { RIGHT, LEFT } };
    pairs[0][1] = ( PiecePair ) { .index = 5, .sides = { LEFT, RIGHT } };
    pairs[1][0] = ( PiecePair ) { .index = 2, .sides = { RIGHT, LEFT } };
    pairs[1][1] = ( PiecePair ) { .index = 6, .sides = { BOTTOM, TOP } };
    pairs[2][0] = ( PiecePair ) { .index = 3, .sides = { RIGHT, LEFT } };
    pairs[2][1] = ( PiecePair ) { .index = 7, .sides = { BOTTOM, TOP } };
    pairs[3][0] = ( PiecePair ) { .index = 4, .sides = { RIGHT, LEFT } };
    pairs[3][1] = ( PiecePair ) { .index = 8, .sides = { BOTTOM, TOP } };
    pairs[4][0] = ( PiecePair ) { .index = 9, .sides = { RIGHT, LEFT } };

    pairs[5][0] = ( PiecePair ) { .index = 6, .sides = { BOTTOM, LEFT } };
    pairs[5][1] = ( PiecePair ) { .index = 10, .sides = { LEFT, RIGHT } };
    pairs[6][0] = ( PiecePair ) { .index = 7, .sides = { RIGHT, LEFT } };
    pairs[6][1] = ( PiecePair ) { .index = 11, .sides = { BOTTOM, TOP } };
    pairs[7][0] = ( PiecePair ) { .index = 8, .sides = { RIGHT, LEFT } };
    pairs[7][1] = ( PiecePair ) { .index = 12, .sides = { BOTTOM, TOP } };
    pairs[8][0] = ( PiecePair ) { .index = 9, .sides = { RIGHT, LEFT } };
    pairs[8][1] = ( PiecePair ) { .index = 13, .sides = { BOTTOM, TOP } };
    pairs[9][0] = ( PiecePair ) { .index = 14, .sides = { RIGHT, LEFT } };

    pairs[10][0] = ( PiecePair ) { .index = 11, .sides = { BOTTOM, LEFT } };
    pairs[10][1] = ( PiecePair ) { .index = 15, .sides = { LEFT, RIGHT } };
    pairs[11][0] = ( PiecePair ) { .index = 12, .sides = { RIGHT, LEFT } };
    pairs[11][1] = ( PiecePair ) { .index = 16, .sides = { BOTTOM, TOP } };
    pairs[12][0] = ( PiecePair ) { .index = 13, .sides = { RIGHT, LEFT } };
    pairs[12][1] = ( PiecePair ) { .index = 17, .sides = { BOTTOM, TOP } };
    pairs[13][0] = ( PiecePair ) { .index = 14, .sides = { RIGHT, LEFT } };
    pairs[13][1] = ( PiecePair ) { .index = 18, .sides = { BOTTOM, TOP } };
    pairs[14][0] = ( PiecePair ) { .index = 19, .sides = { RIGHT, LEFT } };

    pairs[15][0] = ( PiecePair ) { .index = 16, .sides = { BOTTOM, LEFT } };
    pairs[15][1] = ( PiecePair ) { .index = 20, .sides = { LEFT, RIGHT } };
    pairs[16][0] = ( PiecePair ) { .index = 17, .sides = { RIGHT, LEFT } };
    pairs[16][1] = ( PiecePair ) { .index = 21, .sides = { BOTTOM, TOP } };
    pairs[17][0] = ( PiecePair ) { .index = 18, .sides = { RIGHT, LEFT } };
    pairs[17][1] = ( PiecePair ) { .index = 22, .sides = { BOTTOM, TOP } };
    pairs[18][0] = ( PiecePair ) { .index = 19, .sides = { RIGHT, LEFT } };
    pairs[18][1] = ( PiecePair ) { .index = 23, .sides = { BOTTOM, TOP } };
    pairs[19][0] = ( PiecePair ) { .index = 24, .sides = { RIGHT, LEFT } };

    pairs[20][0] = ( PiecePair ) { .index = 21, .sides = { LEFT, RIGHT } };
    pairs[21][0] = ( PiecePair ) { .index = 22, .sides = { LEFT, RIGHT } };
    pairs[22][0] = ( PiecePair ) { .index = 23, .sides = { LEFT, RIGHT } };
    pairs[23][0] = ( PiecePair ) { .index = 24, .sides = { LEFT, RIGHT } };
}

static uint puzzle_calculateOriginalConnections( const Puzzle* const puzzle,
                                                 const PuzzleSolution* const solution,
                                                 const PiecePair pairs[24][2] ) {
    const static char corners[] = { 0, 4, 24, 20 };
    

    uint numOriginalConnections = 0;
    char indexes[25];
    char rotations[25] = {0};
    for ( uint i = 0; i < 4; ++i ) {
        indexes[corners[i]] = solution->edges->cornerIndexes[i];
        if ( i < 3 ) {
            indexes[( i + 1 ) * 5] = solution->edges->leftEdgeIndexes[i];
            indexes[( i + 1 ) * 5 + 4]= solution->edges->rightEdgeIndexes[i];
            indexes[i + 1] = solution->edges->topEdgeIndexes[i];
            indexes[i + 21] = solution->edges->bottomEdgeIndexes[i];
        }
    }

    for ( uint i = i; i < 3; ++i ) {
        for ( uint j = 0; j < 3; ++j ) {
            indexes[( i + 1 ) * 5 + j + 1] = solution->centers->indexes[i][j];
            rotations[( i + 1 ) * 5 + j + 1] = solution->centers->rotations[i][j];
        }
    }


    for ( uint i = 0; i < 25; ++i ) {
    }

    return numOriginalConnections;
}

uint puzzle_findValidSolutions( const Puzzle* const puzzle ) {
    //only 6 valid arangements of corners (top left, top right, bottom right, bottom left)
    const static char cornerArrangements[6][5] = { {0, 4, 20, 24, 0}, {0, 4, 24, 20, 0},
                                                   {0, 20, 4, 24, 0}, {0, 20, 24, 4, 0},
                                                   {0, 24, 4, 20, 0}, {0, 24, 20, 4, 0} };
    static bool pairsGenerated = false;
    PiecePair pairs[24][2] = {-1};
    if ( !pairsGenerated ) {
        puzzle_generateOriginalPiecePairs( pairs );
        pairsGenerated = true;
    }
    //get the valid triplets of edges
    uint numValidEdges = 0;
    const uint maxValidEdges = 400;
    TripleIndex validEdges[maxValidEdges];
    puzzle_calculateValidEdges( puzzle, validEdges, &numValidEdges, maxValidEdges );
    if ( numValidEdges < 4 ) {
        printf( "Error in edge solver\n" );
    }

    //for all of the valid configurations, try all possible combinations of edges
    uint numEdgeSolutions = 0;
    uint maxEdgeSolutions = 7920;
    EdgeSolution edgeSolutions[maxEdgeSolutions];
    for ( uint i = 0; i < 6; ++i ) {
        char edges[4];
        puzzle_recEdgeSolve( puzzle, edges, cornerArrangements[i], 
                             validEdges, numValidEdges, 0, edgeSolutions,
                             &numEdgeSolutions, maxEdgeSolutions );
    }

    uint numTotalConfigurations = 0;
    uint configurations[200][2]; //[0] = edge, [1] = center

    uint numCenterSolutions = 0;
    uint maxCenterSolutions = 2000;
    CenterSolution centerSolutions[maxCenterSolutions];
    for ( uint i = 0; i < numEdgeSolutions; ++i ) {
        uint numValidCenters = 0;
        const uint maxValidCenters = 400;
        TripleIndex validCenters[maxValidCenters];
        puzzle_calculateValidCenterRows( puzzle, &edgeSolutions[i], validCenters,
                                         &numValidCenters, maxValidCenters );
        char centerIndexes[3];
        uint temp = numCenterSolutions;
        puzzle_recCenterSolve( puzzle, centerIndexes, &edgeSolutions[i],
                               validCenters, numValidCenters, 0,
                               centerSolutions, &numCenterSolutions, maxCenterSolutions );
        for  ( uint j = temp; j < numCenterSolutions; ++j ) {
            if ( puzzle_countChanges( &edgeSolutions[i], &centerSolutions[j] ) > 20 ) {
                puzzle_printSolution( &edgeSolutions[i], &centerSolutions[j] );
            }
            configurations[numTotalConfigurations][0] = i;
            configurations[numTotalConfigurations][1] = j;
            ++numTotalConfigurations;
            if ( numTotalConfigurations == 300 ) {
                printf( "Too many solutions\n" );
            }
        }
    }
    /*
    if ( numTotalConfigurations > 1000 ) {
        printf( "----------------\n" );
        for ( uint i = 0; i < numTotalConfigurations; ++i ) {
            puzzle_printSolution( &edgeSolutions[configurations[i][0]], &centerSolutions[configurations[i][1]] );
        }
        printf( "\n----------------\n" );
    }
    */
    printf( "----------------\n" );
    for ( uint i = 0; i < numTotalConfigurations; ++i ) {
        PuzzleSolution temp = ( PuzzleSolution ) { .edges = &edgeSolutions[configurations[i][0]],
                                                   .centers = &centerSolutions[configurations[i][1]] };
        puzzle_printSolution( &edgeSolutions[configurations[i][0]], &centerSolutions[configurations[i][1]] );
        uint numPairs = puzzle_calculateOriginalConnections( puzzle, &temp, pairs );
        printf( "Original pairs: %u/40\n", numPairs );
    }
    printf( "----------------\n" );
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
        if ( i == 0 ) {
            puzzle->pieces[0] = piece_create( CORNER, i, 0, puzzle->connections[0],
                                              0, puzzle->connections[20] );
            puzzle->cornerPieces[cornerIndex++] = &puzzle->pieces[i];
            continue;
        } else if ( i == 4 ) {
            puzzle->pieces[4] = piece_create( CORNER, i, 0, puzzle->connections[24],
                                              0, -puzzle->connections[15] );
            puzzle->cornerPieces[cornerIndex++] = &puzzle->pieces[i];
            continue;
        } else if ( i == 20 ) {
            puzzle->pieces[20] = piece_create( CORNER, i, 0, -puzzle->connections[35],
                                               0, puzzle->connections[4] );
            puzzle->cornerPieces[cornerIndex++] = &puzzle->pieces[i];
            continue;
        } else if ( i == 24 ) {
            puzzle->pieces[24] = piece_create( CORNER, i, 0, -puzzle->connections[19],
                                               0, -puzzle->connections[39] );
            puzzle->cornerPieces[cornerIndex++] = &puzzle->pieces[i];
            continue;
        } else if ( i < 4 ) { //top edge
            puzzle->pieces[i] = piece_create( EDGE, i, 0, puzzle->connections[i * 5],
                                              puzzle->connections[i + 20],
                                              -puzzle->connections[( i - 1 ) * 5] );
            puzzle->edgePieces[edgeIndex++] = &puzzle->pieces[i];
            continue;
        } else if ( i % 5 == 0 ) { //left edge
            puzzle->pieces[i] = piece_create( EDGE, i, 0, -puzzle->connections[i + 15],
                                              puzzle->connections[i / 5],
                                              puzzle->connections[i + 20] );
            puzzle->edgePieces[edgeIndex++] = &puzzle->pieces[i];
            continue;
        } else if ( ( i - 4) % 5 == 0 ) { //right edge
            puzzle->pieces[i] = piece_create( EDGE, i, 0, puzzle->connections[i + 20],
                                              -puzzle->connections[( i + 1 ) / 5 + 14],
                                              -puzzle->connections[i + 15] );
            puzzle->edgePieces[edgeIndex++] = &puzzle->pieces[i];
            continue;
        } else if ( i > 20 && i < 24 ) { //bottom edge
            puzzle->pieces[i] = piece_create( EDGE, i, 0, -puzzle->connections[( i - 21 ) * 5 + 4],
                                              -puzzle->connections[i + 15],
                                              puzzle->connections[( i - 20 ) * 5 + 4] );
            puzzle->edgePieces[edgeIndex++] = &puzzle->pieces[i];
            continue;
        }

        uint row = i / 5;
        uint col = i % 5;
        puzzle->pieces[i] = piece_create( CENTER, i, -puzzle->connections[i + 15],
                                          puzzle->connections[5 * col + row],
                                          puzzle->connections[i + 20],
                                          -puzzle->connections[5 * ( col - 1 ) + row] );
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

void puzzle_free( Puzzle* const puzzle ) {
    if ( !puzzle ) {
        return;
    }
    free( puzzle );
}
