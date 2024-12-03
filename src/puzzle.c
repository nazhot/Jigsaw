#include "puzzle.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pieces.h"
#include "rand.h"


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

typedef struct PiecePair {
    char indexes[2];
    char sides[2];
} PiecePair;


static void puzzle_printSolution( const PuzzleSolution* const solution ) {
    for ( uint i = 0; i < 5; ++i ) {
        uint index = i * 5;
        printf( "%02i %02i %02i %02i %02i\n", solution->indexes[index + 0],
                                              solution->indexes[index + 1],
                                              solution->indexes[index + 2],
                                              solution->indexes[index + 3],
                                              solution->indexes[index + 4] );
    }
    printf( "Rotations:\n" );
    for ( uint i = 0; i < 3; ++i ) {
        uint index = 6 + ( i * 5 );
        printf( "%i %i %i\n", solution->rotations[index], solution->rotations[index + 1],
                              solution->rotations[index + 2] );
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
    static const uint cornerIndexes[4] = { 0, 4, 20, 24 };
    static const uint edgeIndexes[12] = { 1, 2, 3, 5, 10, 15, 9, 14, 19, 21, 22, 23 };
    //Right/Left refer to Right/Left of edge pieces
    char validLefts[4] = {0};
    char validRights[4] = {0};
    for ( uint i = 0; i < 4; ++i ) {
        const Piece corner = puzzle->pieces[cornerIndexes[i]];
        validLefts[i] = -piece_getSide( corner, RIGHT );
        validRights[i] = -piece_getSide( corner, LEFT );
    }

    *numValidEdges = 0;
    for ( uint i = 0; i < 12; ++i ) { //left
        const Piece first = puzzle->pieces[edgeIndexes[i]];
        const char firstLeft = piece_getSide( first, LEFT );

        if ( !charArrayContains( validLefts, 4, firstLeft ) ) {
            continue;
        }

        const char firstRight = piece_getSide( first, RIGHT );

        for ( uint j = 0; j < 12; ++j ) {
            const Piece second = puzzle->pieces[edgeIndexes[j]];
            const char secondLeft = piece_getSide( second, LEFT );

            if ( j == i || secondLeft + firstRight != 0 ) {
                continue;
            }

            const char secondRight = piece_getSide( second, RIGHT );

            for ( uint k = 0; k < 12; ++k ) {
                const Piece third = puzzle->pieces[edgeIndexes[k]];
                const char thirdRight = piece_getSide( third, RIGHT );

                if ( !charArrayContains( validRights, 4, thirdRight ) ) {
                    continue; 
                }

                const char thirdLeft = piece_getSide( third, LEFT );

                if ( i == k || j == k || secondRight + thirdLeft != 0 ) {
                    continue;
                }

                validEdges[*numValidEdges].indexes[0] = first.index;
                validEdges[*numValidEdges].indexes[1] = second.index;
                validEdges[*numValidEdges].indexes[2] = third.index;
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
    static const uint centerIndexes[9] = { 6, 7, 8, 11, 12, 13, 16, 17, 18 };
    //Right/Left refer to Right/Left of edge pieces
    char validLefts[3] = {0};
    char validRights[3] = {0};
    for ( uint i = 0; i < 3; ++i ) {
        const Piece leftEdge = puzzle->pieces[( int ) edgeSolution->leftEdgeIndexes[i]];
        const Piece rightEdge = puzzle->pieces[( int ) edgeSolution->rightEdgeIndexes[i]];
        validLefts[i]= -piece_getSide( leftEdge, BOTTOM );
        validRights[i]= -piece_getSide( rightEdge, BOTTOM );
    }

    *numValidCenterRows = 0;

    for ( uint i = 0; i < 36; ++i ) {
        const uint firstIndex = i / 4; 
        const uint firstRotation = i % 4;
        const Piece firstPiece = puzzle->pieces[centerIndexes[firstIndex]];
        const char firstLeft = piece_getSideWithRotation( firstPiece, LEFT, firstRotation );
        if ( !charArrayContains( validLefts, 3, firstLeft ) ) {
            continue;
        }
        const char firstRight = piece_getSideWithRotation( firstPiece, RIGHT, firstRotation );
        for ( uint j = 0; j < 36; ++j ) {
            const uint secondIndex = j / 4; 
            if ( firstIndex == secondIndex ) {
                continue;
            }
            const uint secondRotation = j % 4;
            const Piece secondPiece = puzzle->pieces[centerIndexes[secondIndex]];
            const char secondLeft = piece_getSideWithRotation( secondPiece, LEFT, secondRotation );
            if ( firstRight + secondLeft != 0 ) {
                continue;
            }
            const char secondRight = piece_getSideWithRotation( secondPiece, RIGHT, secondRotation );
            for ( uint k = 0; k < 36; ++k ) {
                const uint thirdIndex = k / 4; 
                if ( thirdIndex == secondIndex || thirdIndex == firstIndex ) {
                    continue;
                }
                const uint thirdRotation = k % 4;
                const Piece thirdPiece = puzzle->pieces[centerIndexes[thirdIndex]];
                const char thirdLeft = piece_getSideWithRotation( thirdPiece, LEFT, thirdRotation );
                if ( secondRight + thirdLeft != 0 ) {
                    continue;
                }
                const char thirdRight = piece_getSideWithRotation( thirdPiece, RIGHT, thirdRotation );
                if ( !charArrayContains( validRights, 3, thirdRight ) ) {
                    continue;
                }

                validCenterRows[*numValidCenterRows].indexes[0] = firstPiece.index;
                validCenterRows[*numValidCenterRows].indexes[1] = secondPiece.index;
                validCenterRows[*numValidCenterRows].indexes[2] = thirdPiece.index;
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
        } else if ( currentRow == 1 ) {
            TripleIndex rowAbove = centerTriples[( int ) centerIndexes[0]];
            for ( uint j = 0; j < 3; ++j ) {
                if ( piece_getSideWithRotation( puzzle->pieces[( int ) row.indexes[j]], TOP, row.rotations[j] ) +
                    piece_getSideWithRotation( puzzle->pieces[( int ) rowAbove.indexes[j]], BOTTOM, rowAbove.rotations[j] ) != 0 ) {
                    valid = false;
                    break;
                }

            }
        } else if ( currentRow == 2 ) {
            TripleIndex rowAbove = centerTriples[( int ) centerIndexes[1]];
            for ( uint j = 0; j < 3; ++j ) {
                if ( piece_getSideWithRotation( puzzle->pieces[( int ) row.indexes[j]], BOTTOM, row.rotations[j] ) +
                    piece_getSide( puzzle->pieces[( int ) edgeSolution->bottomEdgeIndexes[j]], BOTTOM ) != 0 ) {
                    valid = false;
                    break;
                }
                if ( piece_getSideWithRotation( puzzle->pieces[( int ) row.indexes[j]], TOP, row.rotations[j] ) +
                    piece_getSideWithRotation( puzzle->pieces[( int ) rowAbove.indexes[j]], BOTTOM, rowAbove.rotations[j] ) != 0 ) {
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

static void puzzle_convertEdgeCenterToSolution( PuzzleSolution* const solution,
                                                const EdgeSolution* const edgeSolution,
                                                const CenterSolution* const centerSolution ) {
    const static int corners[] = { 0, 4, 24, 20 };

    for ( uint i = 0; i < 25; ++i ) {
        solution->rotations[i] = 0;
    }

    for ( uint i = 0; i < 4; ++i ) {
        solution->indexes[corners[i]] = edgeSolution->cornerIndexes[i];
        if ( i < 3 ) {
            solution->indexes[( i + 1 ) * 5] = edgeSolution->leftEdgeIndexes[i];
            solution->indexes[( i + 1 ) * 5 + 4]= edgeSolution->rightEdgeIndexes[i];
            solution->indexes[i + 1] = edgeSolution->topEdgeIndexes[i];
            solution->indexes[i + 21] = edgeSolution->bottomEdgeIndexes[i];
        }
    }

    for ( uint i = 0; i < 3; ++i ) {
        for ( uint j = 0; j < 3; ++j ) {
            int index = ( i + 1 ) * 5 + j + 1;
            solution->indexes[index] = centerSolution->indexes[i][j];
            solution->rotations[index] = centerSolution->rotations[i][j];
        }
    }
}

static void puzzle_calculateOriginalConnections( const Puzzle* const puzzle,
                                                 const PuzzleSolution* const solution,
                                                 uint* const numIndexConnections,
                                                 uint* const numSideConnections ) {
    const static SideDirection verticalSides[20][2] = { { RIGHT, LEFT}, { BOTTOM, LEFT }, { BOTTOM, LEFT }, { BOTTOM, LEFT}, { LEFT, RIGHT },
                                                        { RIGHT, LEFT }, { RIGHT, LEFT }, { RIGHT, LEFT }, { RIGHT, LEFT }, { LEFT, RIGHT },
                                                        { RIGHT, LEFT }, { RIGHT, LEFT }, { RIGHT, LEFT }, { RIGHT, LEFT }, { LEFT, RIGHT },
                                                        { RIGHT, LEFT }, { RIGHT, BOTTOM }, { RIGHT, BOTTOM }, { RIGHT, BOTTOM }, { LEFT, RIGHT } };

    const static SideDirection horizontalSides[20][2] = { { LEFT, RIGHT }, { BOTTOM, TOP }, { BOTTOM, TOP }, { BOTTOM, TOP }, { RIGHT, LEFT },
                                                          { LEFT, RIGHT }, { BOTTOM, TOP }, { BOTTOM, TOP }, { BOTTOM, TOP }, { RIGHT, LEFT },
                                                          { LEFT, RIGHT }, { BOTTOM, TOP }, { BOTTOM, TOP }, { BOTTOM, TOP }, { RIGHT, LEFT },
                                                          { LEFT, RIGHT }, { BOTTOM, BOTTOM }, { BOTTOM, BOTTOM }, { BOTTOM, BOTTOM }, { RIGHT, LEFT } };
    
    *numIndexConnections = 0;
    *numSideConnections = 0;

    PiecePair originalPairs[40];
    uint originalIndex = 0;
    PiecePair solutionPairs[40];
    uint solutionIndex = 0;
    uint sidesIndex = 0;
    for ( uint col = 0; col < 4; ++col ) {
        for ( uint row = 0; row < 5; ++row ) {
            int index = row * 5 + col;
            originalPairs[originalIndex++] = ( PiecePair ) { .indexes = { index, index + 1 },
                                                             .sides = { verticalSides[sidesIndex][0], verticalSides[sidesIndex][1] } };
            bool swap = solution->indexes[index] > solution->indexes[index + 1];
            int minIndex = swap ? solution->indexes[index + 1] : solution->indexes[index];
            int maxIndex = swap ? solution->indexes[index] : solution->indexes[index + 1];
            solutionPairs[solutionIndex] = ( PiecePair ) { .indexes = { minIndex, maxIndex },
                                                             .sides = { verticalSides[sidesIndex][swap ? 1 : 0], verticalSides[sidesIndex][swap ? 0 : 1] } };
            solutionPairs[solutionIndex].sides[0] += ( 4 - solution->rotations[swap ? index + 1 : index] );
            solutionPairs[solutionIndex].sides[0] %= 4;
            solutionPairs[solutionIndex].sides[1] += ( 4 - solution->rotations[swap ? index : index + 1] );
            solutionPairs[solutionIndex].sides[1] %= 4;
            ++solutionIndex;
            ++sidesIndex;
        }
    }

    sidesIndex = 0;
    for ( uint row = 0; row < 4; ++row ) {
        for ( uint col = 0; col < 5; ++col ) {
            int index = row * 5 + col;
            originalPairs[originalIndex++] = ( PiecePair ) { .indexes = { index, index + 5 },
                                                             .sides = { horizontalSides[sidesIndex][0], horizontalSides[sidesIndex][1] } };
            bool swap = solution->indexes[index] > solution->indexes[index + 5];
            int minIndex = swap ? solution->indexes[index + 5] : solution->indexes[index];
            int maxIndex = swap ? solution->indexes[index] : solution->indexes[index + 5];
            solutionPairs[solutionIndex] = ( PiecePair ) { .indexes = { minIndex, maxIndex },
                                                             .sides = { horizontalSides[sidesIndex][swap ? 1 : 0], horizontalSides[sidesIndex][swap ? 0 : 1] } };
            solutionPairs[solutionIndex].sides[0] += ( 4 - solution->rotations[swap ? index + 5 : index] );
            solutionPairs[solutionIndex].sides[0] %= 4;
            solutionPairs[solutionIndex].sides[1] += ( 4 - solution->rotations[swap ? index : index + 5] );
            solutionPairs[solutionIndex].sides[1] %= 4;
            ++solutionIndex;
            ++sidesIndex;
        }
    }


    for ( uint i = 0; i < 40; ++i ) {
        for ( uint j = 0; j < 40; ++j ) {
            if ( originalPairs[i].indexes[0] == solutionPairs[j].indexes[0] &&
                 originalPairs[i].indexes[1] == solutionPairs[j].indexes[1] ) {
                ++*numIndexConnections;
                if ( originalPairs[i].sides[0] == solutionPairs[j].sides[0] &&
                     originalPairs[i].sides[1] == solutionPairs[j].sides[1] ) {
                    ++*numSideConnections;
                }
                break;      
            }
        }
    }
}

void puzzle_findValidSolutions( const Puzzle* const puzzle,
                                PuzzleSolution* const otherSolutions,
                                uint* const numOtherSolutions, const uint maxOtherSolutions,
                                uint* const maxUniqueIndexes, uint* const maxUniqueSides ) {
    //only 6 valid arangements of corners (top left, top right, bottom right, bottom left)
    const static char cornerArrangements[6][5] = { {0, 4, 20, 24, 0}, {0, 4, 24, 20, 0},
                                                   {0, 20, 4, 24, 0}, {0, 20, 24, 4, 0},
                                                   {0, 24, 4, 20, 0}, {0, 24, 20, 4, 0} };
    //get the valid triplets of edges
    uint numValidEdges = 0;
    const uint maxValidEdges = 2000;
    TripleIndex validEdges[maxValidEdges];
    puzzle_calculateValidEdges( puzzle, validEdges, &numValidEdges, maxValidEdges );
    if ( numValidEdges < 4 ) {
        printf( "Error in edge solver\n" );
    }

    //for all of the valid configurations, try all possible combinations of edges
    uint numEdgeSolutions = 0;
    uint maxEdgeSolutions = 10000;
    EdgeSolution edgeSolutions[maxEdgeSolutions];
    for ( uint i = 0; i < 6; ++i ) {
        char edges[4];
        puzzle_recEdgeSolve( puzzle, edges, cornerArrangements[i], 
                             validEdges, numValidEdges, 0, edgeSolutions,
                             &numEdgeSolutions, maxEdgeSolutions );
    }

    uint numTotalConfigurations = 0;
    uint maxTotalConfigurations = 300;
    uint configurations[maxTotalConfigurations][2]; //[0] = edge, [1] = center

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
            configurations[numTotalConfigurations][0] = i;
            configurations[numTotalConfigurations][1] = j;
            ++numTotalConfigurations;
            if ( numTotalConfigurations == maxTotalConfigurations ) {
                printf( "Too many solutions\n" );
            }
        }
    }

    *numOtherSolutions = 0;
    *maxUniqueIndexes = 0;
    *maxUniqueSides = 0;
    for ( uint i = 0; i < numTotalConfigurations; ++i ) {
        PuzzleSolution temp;
        puzzle_convertEdgeCenterToSolution( &temp, &edgeSolutions[configurations[i][0]],
                                            &centerSolutions[configurations[i][1]] );
        uint numIndexConnections = 0;
        uint numSideConnections = 0;
        puzzle_calculateOriginalConnections( puzzle, &temp,
                                             &numIndexConnections,
                                             &numSideConnections );
        if ( numIndexConnections < 40 ) {
            otherSolutions[*numOtherSolutions] = temp;
            ++*numOtherSolutions;;
            if ( ( 40 - numIndexConnections ) > *maxUniqueIndexes ) {
                *maxUniqueIndexes = 40 - numIndexConnections;
            }
            if ( ( 40 - numSideConnections ) > *maxUniqueSides ) {
                *maxUniqueSides = 40 - numSideConnections;
            }
            if ( *numOtherSolutions == maxOtherSolutions ) {
                fprintf( stderr, "Too many total solutions\n" );
                exit( 1 );
            }
        }
    }
}

static void puzzle_setPieces( Puzzle* const puzzle ) {
    for ( uint i = 0; i < 25; ++i ) {
        if ( i == 0 ) {
            puzzle->pieces[0] = piece_create( CORNER, i, 0, puzzle->connections[0],
                                              0, puzzle->connections[20] );
        } else if ( i == 4 ) {
            puzzle->pieces[4] = piece_create( CORNER, i, 0, puzzle->connections[24],
                                              0, -puzzle->connections[15] );
        } else if ( i == 20 ) {
            puzzle->pieces[20] = piece_create( CORNER, i, 0, -puzzle->connections[35],
                                               0, puzzle->connections[4] );
        } else if ( i == 24 ) {
            puzzle->pieces[24] = piece_create( CORNER, i, 0, -puzzle->connections[19],
                                               0, -puzzle->connections[39] );
        } else if ( i < 4 ) { //top edge
            puzzle->pieces[i] = piece_create( EDGE, i, 0, puzzle->connections[i * 5],
                                              puzzle->connections[i + 20],
                                              -puzzle->connections[( i - 1 ) * 5] );
        } else if ( i % 5 == 0 ) { //left edge
            puzzle->pieces[i] = piece_create( EDGE, i, 0, -puzzle->connections[i + 15],
                                              puzzle->connections[i / 5],
                                              puzzle->connections[i + 20] );
        } else if ( ( i - 4) % 5 == 0 ) { //right edge
            puzzle->pieces[i] = piece_create( EDGE, i, 0, puzzle->connections[i + 20],
                                              -puzzle->connections[( i + 1 ) / 5 + 14],
                                              -puzzle->connections[i + 15] );
        } else if ( i > 20 && i < 24 ) { //bottom edge
            puzzle->pieces[i] = piece_create( EDGE, i, 0, -puzzle->connections[( i - 21 ) * 5 + 4],
                                              -puzzle->connections[i + 15],
                                              puzzle->connections[( i - 20 ) * 5 + 4] );
        } else {
            uint row = i / 5;
            uint col = i % 5;
            puzzle->pieces[i] = piece_create( CENTER, i, -puzzle->connections[i + 15],
                                              puzzle->connections[5 * col + row],
                                              puzzle->connections[i + 20],
                                              -puzzle->connections[5 * ( col - 1 ) + row] );
        }
    }
}

void puzzle_shuffle( Puzzle* const puzzle ) {
    const uint numUniqueConnectors = puzzle->numUniqueConnectors;
    const uint numEach = 40 / numUniqueConnectors;
    const uint numLeftOver = 40 - numUniqueConnectors * numEach;

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
    
    puzzle_setPieces( puzzle );
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

void puzzle_mutate( Puzzle* const destPuzzle, const Puzzle* const srcPuzzle,
                    const uint minMutations, const uint maxMutations ) {
    memcpy( destPuzzle, srcPuzzle, sizeof( Puzzle ) );
    uint numMutations = rand_intBetween( minMutations, maxMutations + 1 );
    while ( numMutations ) {
        uint firstIndex = 0;
        uint secondIndex = 0;
        while ( firstIndex == secondIndex ) {
            firstIndex = rand_index( 40 );
            secondIndex = rand_index( 40 );
        }
        char temp = destPuzzle->connections[firstIndex];
        destPuzzle->connections[firstIndex] = destPuzzle->connections[secondIndex];
        destPuzzle->connections[secondIndex] = temp;
        --numMutations;
        if ( numMutations == 0 ) {
            if ( memcmp( srcPuzzle->connections, destPuzzle->connections, sizeof( char ) * 40 ) == 0 ) {
                numMutations = 1;
            }
        }
    }
    puzzle_setPieces( destPuzzle );
}

typedef struct PuzzleSum {
    Puzzle* puzzle;
    uint sum;
    uint numUniqueSides;
    uint numUniqueIndexes;
} PuzzleSum;

static int puzzleSumSortDescending( const void* p1, const void* p2 ) {
    PuzzleSum* puzzleSum1 = ( PuzzleSum* ) p1;
    PuzzleSum* puzzleSum2 = ( PuzzleSum* ) p2;
    return puzzleSum2->sum - puzzleSum1->sum;
}

static int puzzleSidesSortDescending( const void* p1, const void* p2 ) {
    PuzzleSum* puzzleSum1 = ( PuzzleSum* ) p1;
    PuzzleSum* puzzleSum2 = ( PuzzleSum* ) p2;
    return puzzleSum2->numUniqueSides - puzzleSum1->numUniqueSides;
}

static int puzzleIndexesSortDescending( const void* p1, const void* p2 ) {
    PuzzleSum* puzzleSum1 = ( PuzzleSum* ) p1;
    PuzzleSum* puzzleSum2 = ( PuzzleSum* ) p2;
    return puzzleSum2->numUniqueIndexes - puzzleSum1->numUniqueIndexes;
}

void puzzle_findMostUniqueSolution( const uint numUniqueConnections,
                                    const uint generationSize,
                                    const uint numGenerations,
                                    const uint numSurvivors, const uint numChildren,
                                    const uint minMutations, const uint maxMutations ) {
    PuzzleSum* generation = malloc( sizeof( PuzzleSum ) * generationSize );
    for ( uint i = 0; i < generationSize; ++i ) {
        generation[i].puzzle = puzzle_create( numUniqueConnections ); 
        generation[i].sum = 0;
    }

     
    uint bestUniqueSums = 0;
    uint bestComparison = 0;
    bool foundBestSides = false;
    for ( uint i = 0; i < numGenerations; ++i ) {
        uint bestInGeneration = 0;
        PuzzleSolution best;
        uint totalSum = 0;
        for ( uint j = 0; j < generationSize; ++j ) {
            uint maxUniqueIndexes = 0;
            uint maxUniqueSides = 0;
            uint maxOtherSolutions = 100;
            uint numOtherSolutions = 0;
            PuzzleSolution solutions[maxOtherSolutions];
            puzzle_findValidSolutions( generation[j].puzzle, solutions,
                                       &numOtherSolutions, maxOtherSolutions,
                                       &maxUniqueIndexes, &maxUniqueSides );
            if ( numOtherSolutions != 1 ) {
                generation[j].sum = 0;
                generation[j].numUniqueSides = 0;
                generation[j].numUniqueIndexes = 0;
                continue;
            }
            uint sum = maxUniqueSides + maxUniqueIndexes;
            uint comparison = foundBestSides ? sum : maxUniqueSides;
            if ( comparison > bestInGeneration ) {
                bestInGeneration = comparison;
                best = solutions[0];
            }
            totalSum += sum;

            generation[j].sum = sum;
            generation[j].numUniqueSides = maxUniqueSides;
            generation[j].numUniqueIndexes = maxUniqueIndexes;
        }

        if ( foundBestSides ) {
            qsort( generation, generationSize, sizeof( PuzzleSum ), puzzleSumSortDescending );
        } else {
            qsort( generation, generationSize, sizeof( PuzzleSum ), puzzleSidesSortDescending );
        }
        uint comparison = foundBestSides ? generation[0].sum : generation[0].numUniqueSides;
        if ( comparison > bestComparison || generation[0].numUniqueSides == 40 ) {
            bestComparison = comparison;
            if ( !foundBestSides && comparison == 40 ) {
                foundBestSides = true;
            }
            printf( "Starting Generation: %u/%u\n", i + 1, numGenerations );
            puzzle_printSolution( &best );
            printf( "Best Sum of Uniques: %u\n", generation[0].sum );
            printf( "Unique Sides: %u\n", generation[0].numUniqueSides );
            printf( "Unique Indexes: %u\n", generation[0].numUniqueIndexes );
            printf( "Average: %.2f\n", totalSum * 1.0 / generationSize ); 
            float last100Average = 0;
            for ( uint i = 0; i < 100; ++i ) {
                last100Average += generation[i].sum;
            }
            last100Average /= 100.0;
            printf( "Last 100 Average: %.2f\n", last100Average );
            for ( uint j = 0; j < 40; ++j ) {
                if ( j ) {
                    printf( ", " );
                }
                printf( "%i", generation[0].puzzle->connections[j] );
            }
            printf( "\n" );
        }

        uint index = numSurvivors;
        for ( uint j = 0; j < numSurvivors; ++j ) {
            for ( uint k = 0; k < numChildren; ++k ) {
                puzzle_mutate( generation[index].puzzle, generation[j].puzzle,
                               minMutations, maxMutations );
                ++index;
            }
            puzzle_shuffle( generation[j].puzzle );
        }
        for ( uint j = index; j < generationSize; ++j ) {
            puzzle_shuffle( generation[j].puzzle );
        }
    }
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
