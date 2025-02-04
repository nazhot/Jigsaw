#include "puzzle.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "da.h"
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


bool twoIndexesOriginallyTouched( const char index1, const char index2 ) {
    uint col1 = index1 % 5;
    //don't need to check for row, row0 would be negative number, row4 above 24
    if ( index2 == index1 - 5 || index2 == index1 + 5 ) {
        return true;
    }

    if ( col1 > 0 ) {
        if ( index2 == index1 - 1 ) {
            return true;
        }
    }

    if ( col1 < 4 ) {
        if ( index2 == index1 + 1 ) {
            return true;
        }
    }

    return false;
}

static bool edgeRowIsUnique( const char corner1, const char corner2, const char edges[3] ) {
    if ( twoIndexesOriginallyTouched( corner1, edges[0] ) ) {
        return false;
    }
    if ( twoIndexesOriginallyTouched( edges[0], edges[1] ) ) {
        return false;
    }
    if ( twoIndexesOriginallyTouched( edges[1], edges[2] ) ) {
        return false;
    }
    if ( twoIndexesOriginallyTouched( corner2, edges[2] ) ) {
        return false;
    }
    return true; 
}

static bool edgeSolutionIsUnique( const EdgeSolution* edge ) {
    if ( !edgeRowIsUnique( edge->cornerIndexes[0], edge->cornerIndexes[1], edge->topEdgeIndexes ) ) {
        return false; 
    }
    if ( !edgeRowIsUnique( edge->cornerIndexes[1], edge->cornerIndexes[3], edge->rightEdgeIndexes ) ) {
        return false; 
    }
    if ( !edgeRowIsUnique( edge->cornerIndexes[3], edge->cornerIndexes[2], edge->bottomEdgeIndexes ) ) {
        return false; 
    }
    if ( !edgeRowIsUnique( edge->cornerIndexes[2], edge->cornerIndexes[0], edge->leftEdgeIndexes ) ) {
        return false; 
    }

    return true;
}

void puzzle_printSolution( const PuzzleSolution* const solution ) {
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

static void printEdgeSolution( const EdgeSolution* edge ) {
    printf( "Edge Solution:\n" );
    printf( "%02i %02i %02i %02i %02i\n", edge->cornerIndexes[0], edge->topEdgeIndexes[0], edge->topEdgeIndexes[1], edge->topEdgeIndexes[2], edge->cornerIndexes[1] );
    printf( "%02i          %02i\n", edge->leftEdgeIndexes[2], edge->rightEdgeIndexes[0] );
    printf( "%02i          %02i\n", edge->leftEdgeIndexes[1], edge->rightEdgeIndexes[1] );
    printf( "%02i          %02i\n", edge->leftEdgeIndexes[0], edge->rightEdgeIndexes[2] );
    printf( "%02i %02i %02i %02i %02i\n", edge->cornerIndexes[2], edge->bottomEdgeIndexes[2], edge->bottomEdgeIndexes[1], edge->bottomEdgeIndexes[0], edge->cornerIndexes[3] );
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

static bool uintArrayContains( const uint* const array, const uint arraySize,
                              const uint value ) {
    for ( uint i = 0; i < arraySize; ++i ) {
        if ( array[i] == value ) {
            return true;
        }
    }
    return false;
}

static void puzzle_calculateValidEdgesStack( const Puzzle* const puzzle,
                                             DynamicArray* const validEdges ) {
    static const uint cornerIndexes[4] = { 0, 4, 20, 24 };
    static const uint edgeIndexes[12] = { 1, 2, 3, 5, 10, 15, 9, 14, 19, 21, 22, 23 };
    //Right/Left refer to Right/Left of edge pieces
    char validLefts[4] = {0};
    char validRights[4] = {0};
    for ( uint i = 0; i < 4; ++i ) {
        const Piece corner = puzzle->pieces[cornerIndexes[i]];
        //validLefts[i] = -piece_getSide( corner, RIGHT );
        //validRights[i] = -piece_getSide( corner, LEFT );
        validLefts[i] = piece_getSide( corner, RIGHT );
        validRights[i] = piece_getSide( corner, LEFT );
    }

    validEdges->numElements = 0;
    typedef struct {
        char lastRight;
        uint indexes[3];
        uint numEdges;
    } StackParams;

    static StackParams stack[5000];
    int stackSize = 1;
    stack[0] = ( StackParams ) { .numEdges = 0 };

    while ( stackSize > 0 ) {
        StackParams parameters = stack[--stackSize];
        for ( uint i = 0; i < 12; ++i ) {
            if ( parameters.numEdges &&
                 uintArrayContains( parameters.indexes, parameters.numEdges, i ) ) {
                continue;
            }

            const Piece piece = puzzle->pieces[edgeIndexes[i]];
            const char leftSide = piece_getSide( piece, LEFT );

            if ( parameters.numEdges == 0 ) {
                if ( !charArrayContains( validLefts, 4, leftSide ) ) {
                    continue;
                }
            } else if ( !piece_piecesConnect( leftSide, parameters.lastRight ) ) {
                continue;
            }

            const char rightSide = piece_getSide( piece, RIGHT );

            if ( parameters.numEdges == 2 ) {
                if ( !charArrayContains( validRights, 4, rightSide ) ) {
                    continue;
                }
                parameters.indexes[2] = i;
                TripleIndex tempTriple;
                for ( uint j = 0; j < 3; ++j ) {
                    tempTriple.indexes[j] = edgeIndexes[parameters.indexes[j]];
                }
                da_addElement( validEdges, &tempTriple );
                continue;
            }
            parameters.indexes[parameters.numEdges] = i;
            stack[stackSize] = ( StackParams ) { .lastRight = rightSide,
                                                 .numEdges = parameters.numEdges + 1 };
            memcpy( stack[stackSize].indexes, parameters.indexes, sizeof( uint ) * 3 );
            ++stackSize;
        }
    }
}

static void puzzle_calculateValidEdges( const Puzzle* const puzzle,
                                        DynamicArray* const validEdges ) {
    static const uint cornerIndexes[4] = { 0, 4, 20, 24 };
    static const uint edgeIndexes[12] = { 1, 2, 3, 5, 10, 15, 9, 14, 19, 21, 22, 23 };
    //Right/Left refer to Right/Left of edge pieces
    char validLefts[4] = {0};
    char validRights[4] = {0};
    for ( uint i = 0; i < 4; ++i ) {
        const Piece corner = puzzle->pieces[cornerIndexes[i]];
        //validLefts[i] = -piece_getSide( corner, RIGHT );
        //validRights[i] = -piece_getSide( corner, LEFT );
        validLefts[i] = piece_getSide( corner, RIGHT );
        validRights[i] = piece_getSide( corner, LEFT );
    }

    validEdges->numElements = 0;
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
}

static void puzzle_recEdgeSolve( const Puzzle* const puzzle, uint edgeIndexes[4],
                                const char* const currentArrangement,
                                const DynamicArray* const edgeTriples, 
                                const uint currentEdge, DynamicArray* const edgeSolutions ) {
    const char leftCorner = piece_getSide( puzzle->pieces[( int ) currentArrangement[( int ) currentEdge]], RIGHT );
    const char rightCorner = piece_getSide( puzzle->pieces[( int ) currentArrangement[( int  ) currentEdge + 1]], LEFT );

    for ( uint i = 0; i < edgeTriples->numElements; ++i ) {
        if ( uintArrayContains( edgeIndexes, currentEdge, i ) ) {
            continue;   
        }

        TripleIndex* edgeToCheck = ( TripleIndex* ) da_getElement( edgeTriples, i );
        bool valid = true;
        for ( uint j = 0; j < currentEdge; ++j ) {
            TripleIndex* tempTripleIndex1 = ( TripleIndex* ) da_getElement( edgeTriples, ( int ) edgeIndexes[j] );
            for ( uint k = 0; k < 3; ++k ) {
                if ( charArrayContains( tempTripleIndex1->indexes, 3, edgeToCheck->indexes[k] ) ) {
                    valid = false;
                }
            }
            if ( !valid ) {
                break;
            }
        }
        if ( !valid ) {
            continue;
        }
        const char leftEdge = piece_getSide( puzzle->pieces[( int  ) edgeToCheck->indexes[0]], LEFT );
        
        if ( !piece_piecesConnect( leftEdge, leftCorner ) ) {
            continue;
        }
        const char rightEdge = piece_getSide( puzzle->pieces[( int ) edgeToCheck->indexes[2]], RIGHT );

        if ( !piece_piecesConnect( rightEdge, rightCorner ) ) {
            continue;
        }

        edgeIndexes[currentEdge] = i;
        if ( currentEdge == 3 ) {
            EdgeSolution tempEdgeSolution;
            TripleIndex* topEdge = ( TripleIndex* ) da_getElement( edgeTriples, ( int ) edgeIndexes[0] );
            TripleIndex* rightEdge = ( TripleIndex* ) da_getElement( edgeTriples, ( int ) edgeIndexes[1] );
            TripleIndex* bottomEdge = ( TripleIndex* ) da_getElement( edgeTriples, ( int ) edgeIndexes[2] );
            TripleIndex* leftEdge = ( TripleIndex* ) da_getElement( edgeTriples, ( int ) edgeIndexes[3] );
            for ( uint i = 0; i < 4; ++i ) {
                tempEdgeSolution.cornerIndexes[i] = currentArrangement[i]; 
                if ( i < 3 ) {
                    tempEdgeSolution.topEdgeIndexes[i] = topEdge->indexes[i];
                    tempEdgeSolution.rightEdgeIndexes[i] = rightEdge->indexes[i];   
                    tempEdgeSolution.bottomEdgeIndexes[i] = bottomEdge->indexes[2 - i];   
                    tempEdgeSolution.leftEdgeIndexes[i] = leftEdge->indexes[2 - i];   
                }
            }
            da_addElement( edgeSolutions, &tempEdgeSolution );
        } else {
            puzzle_recEdgeSolve( puzzle, edgeIndexes, currentArrangement, 
                                edgeTriples, currentEdge + 1,
                                edgeSolutions );
        }
    }
}

static void puzzle_calculateValidCenterRowsNoEdge( const Puzzle* const puzzle,
                                                   DynamicArray* const validCenterRows ) {
    static const uint centerIndexes[9] = { 6, 7, 8, 11, 12, 13, 16, 17, 18 };

    validCenterRows->numElements = 0;

    for ( uint i = 0; i < 36; ++i ) {
        const uint firstIndex = i / 4; 
        const uint firstRotation = i % 4;
        const Piece firstPiece = puzzle->pieces[centerIndexes[firstIndex]];
        const char firstRight = piece_getSideWithRotation( firstPiece, RIGHT, firstRotation );
        for ( uint j = 0; j < 36; ++j ) {
            const uint secondIndex = j / 4; 
            if ( firstIndex == secondIndex ) {
                continue;
            }
            const uint secondRotation = j % 4;
            const Piece secondPiece = puzzle->pieces[centerIndexes[secondIndex]];
            if ( secondRotation == 0 && !piece_contains( secondPiece, firstRight ) ) {
                j += 3;
                continue;
            }
            const char secondLeft = piece_getSideWithRotation( secondPiece, LEFT, secondRotation );
            if ( !piece_piecesConnect( firstRight, secondLeft ) ) {
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
                if ( thirdRotation == 0 && !piece_contains( thirdPiece, secondRight ) ) {
                    k += 3;
                    continue;
                }
                const char thirdLeft = piece_getSideWithRotation( thirdPiece, LEFT, thirdRotation );

                if ( !piece_piecesConnect( secondRight, thirdLeft ) ){
                    continue;
                }
                TripleIndex tempValidCenterRow;

                tempValidCenterRow.indexes[0] = firstPiece.index;
                tempValidCenterRow.indexes[1] = secondPiece.index;
                tempValidCenterRow.indexes[2] = thirdPiece.index;
                tempValidCenterRow.rotations[0] = firstRotation;
                tempValidCenterRow.rotations[1] = secondRotation;
                tempValidCenterRow.rotations[2] = thirdRotation;
                da_addElement( validCenterRows, &tempValidCenterRow );
            }
        }
    }
}

static void puzzle_calculateValidCenterRows( const Puzzle* const puzzle,
                                            const EdgeSolution* edgeSolution,
                                            DynamicArray* const validCenterRows,
                                            const uint validNeighbors[9][9],
                                            const uint validNeighborsCount[9] ) {
    static const uint centerIndexes[9] = { 6, 7, 8, 11, 12, 13, 16, 17, 18 };

    //Right/Left refer to Right/Left of edge pieces
    char validLefts[3] = {0};
    char validRights[3] = {0};
    for ( uint i = 0; i < 3; ++i ) {
        const Piece leftEdge = puzzle->pieces[( int ) edgeSolution->leftEdgeIndexes[i]];
        const Piece rightEdge = puzzle->pieces[( int ) edgeSolution->rightEdgeIndexes[i]];
        //validLefts[i]= -piece_getSide( leftEdge, BOTTOM );
        //validRights[i]= -piece_getSide( rightEdge, BOTTOM );
        validLefts[i]= piece_getSide( leftEdge, BOTTOM );
        validRights[i]= piece_getSide( rightEdge, BOTTOM );
    }

    validCenterRows->numElements = 0;

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
            const uint secondIndex = j/4;
            if ( firstIndex == secondIndex ) {
                continue;
            }
            const uint secondRotation = j % 4;
            const Piece secondPiece = puzzle->pieces[centerIndexes[secondIndex]];
            if ( secondRotation == 0 && !piece_contains( secondPiece, firstRight ) ) {
                j += 3;
                continue;
            }
            const char secondLeft = piece_getSideWithRotation( secondPiece, LEFT, secondRotation );
            if ( !piece_piecesConnect( firstRight, secondLeft ) ) {
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
                if ( thirdRotation == 0 && !piece_contains( thirdPiece, secondRight ) ) {
                    k += 3;
                    continue;
                }
                const char thirdLeft = piece_getSideWithRotation( thirdPiece, LEFT, thirdRotation );

                if ( !piece_piecesConnect( secondRight, thirdLeft ) ){
                    continue;
                }
                const char thirdRight = piece_getSideWithRotation( thirdPiece, RIGHT, thirdRotation );
                //make sure that the two edges line up with each other
                //left and right are both top -> bottom as shown in function above this one
                bool valid = false;
                for ( uint l = 0; l < 3; ++l ) {
                    if ( piece_piecesConnect( firstLeft,validLefts[l] ) &&
                         piece_piecesConnect( thirdRight, validRights[l] ) ) {
                        valid = true;
                        break;
                    }
                }

                if (!valid ) { // !charArrayContains( validRights, 3, thirdRight ) ) {
                    continue;
                }

                TripleIndex tempValidCenterRow;

                tempValidCenterRow.indexes[0] = firstPiece.index;
                tempValidCenterRow.indexes[1] = secondPiece.index;
                tempValidCenterRow.indexes[2] = thirdPiece.index;
                tempValidCenterRow.rotations[0] = firstRotation;
                tempValidCenterRow.rotations[1] = secondRotation;
                tempValidCenterRow.rotations[2] = thirdRotation;
                da_addElement( validCenterRows, &tempValidCenterRow );
            }
        }
    }
}

void puzzle_recCenterSolve( const Puzzle* const puzzle, uint centerIndexes[3],
                           const EdgeSolution* const edgeSolution,
                           const DynamicArray* const centerRows, const uint currentRow,
                           DynamicArray* const centerSolutions ) {
    if ( currentRow == 3 ) {
        CenterSolution tempCenterSolution;
        for ( uint i = 0; i < 3; ++i ) {
            TripleIndex* row = ( TripleIndex* ) da_getElement( centerRows, centerIndexes[i] );
            for ( uint j = 0; j < 3; ++j ) {
                tempCenterSolution.indexes[i][j] = row->indexes[j];
                tempCenterSolution.rotations[i][j] = row->rotations[j];
            }
        }
        da_addElement( centerSolutions, &tempCenterSolution );

        return;
    }

    const char leftEdge = piece_getSide( puzzle->pieces[( int ) edgeSolution->leftEdgeIndexes[currentRow]], BOTTOM );
    const char rightEdge = piece_getSide( puzzle->pieces[( int ) edgeSolution->rightEdgeIndexes[currentRow]], BOTTOM );

    for ( int i = 0; i < centerRows->numElements; ++i ) {
        if ( uintArrayContains( centerIndexes, currentRow, i ) ) {
            continue;   
        }

        const TripleIndex* row = ( TripleIndex* ) da_getElement( centerRows, i );

        bool valid = true;
        for ( uint j = 0; j < currentRow; ++j ) {
            TripleIndex* checkRow = ( TripleIndex* ) da_getElement( centerRows, ( int ) centerIndexes[j] );
            for ( uint k = 0; k < 3; ++k ) {
                if ( charArrayContains( checkRow->indexes, 3, row->indexes[k] ) ) {
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
        const char leftCenter = piece_getSide( puzzle->pieces[( int ) row->indexes[0]], LEFT );
        if ( !piece_piecesConnect( leftCenter, leftEdge ) ) {
            continue;
        }
        const char rightCenter = piece_getSide( puzzle->pieces[( int ) row->indexes[2]], RIGHT );
        if ( !piece_piecesConnect( rightCenter, rightEdge ) ) {
            continue;
        }

        if ( currentRow == 0 ) {
            for ( uint j = 0; j < 3; ++j ) {
                if ( !piece_piecesConnect( piece_getSideWithRotation( puzzle->pieces[( int ) row->indexes[j]], TOP, row->rotations[j] ),
                    piece_getSide( puzzle->pieces[( int ) edgeSolution->topEdgeIndexes[j]], BOTTOM ) ) ) {
                    valid = false;
                    break;
                }
            }
        } else if ( currentRow == 1 ) {
            TripleIndex* rowAbove = ( TripleIndex* ) da_getElement( centerRows, ( int ) centerIndexes[0] );
            for ( uint j = 0; j < 3; ++j ) {
                if ( !piece_piecesConnect( piece_getSideWithRotation( puzzle->pieces[( int ) row->indexes[j]], TOP, row->rotations[j] ),
                    piece_getSideWithRotation( puzzle->pieces[( int ) rowAbove->indexes[j]], BOTTOM, rowAbove->rotations[j] ) ) ) {
                    valid = false;
                    break;
                }

            }
        } else if ( currentRow == 2 ) {
            TripleIndex* rowAbove = ( TripleIndex* ) da_getElement( centerRows, ( int ) centerIndexes[1] );
            for ( uint j = 0; j < 3; ++j ) {
                if ( !piece_piecesConnect( piece_getSideWithRotation( puzzle->pieces[( int ) row->indexes[j]], BOTTOM, row->rotations[j] ),
                    piece_getSide( puzzle->pieces[( int ) edgeSolution->bottomEdgeIndexes[j]], BOTTOM ) ) ) {
                    valid = false;
                    break;
                }
                if ( !piece_piecesConnect( piece_getSideWithRotation( puzzle->pieces[( int ) row->indexes[j]], TOP, row->rotations[j] ),
                    piece_getSideWithRotation( puzzle->pieces[( int ) rowAbove->indexes[j]], BOTTOM, rowAbove->rotations[j] ) ) ) {
                    valid = false;
                    break;
                }
            }
        }
        if ( !valid ) {
            continue;
        }

        centerIndexes[currentRow] = i;
        puzzle_recCenterSolve( puzzle, centerIndexes, edgeSolution, centerRows,
                               currentRow + 1, centerSolutions );
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


void puzzle_findValidEdges( const Puzzle* const puzzle, DynamicArray* const edgeSolutions ) {
    //only 6 valid arangements of corners (top left, top right, bottom right, bottom left)
    const static char cornerArrangements[6][5] = { {0, 4, 20, 24, 0}, {0, 4, 24, 20, 0},
        {0, 20, 4, 24, 0}, {0, 20, 24, 4, 0},
        {0, 24, 4, 20, 0}, {0, 24, 20, 4, 0} };
    //get the valid triplets of edges
    static bool allocatedEdges = false;
    static DynamicArray* validEdges;
    if ( !allocatedEdges ) {
        validEdges = da_create( 2000, sizeof( TripleIndex ) );
        allocatedEdges = true;
    }

    puzzle_calculateValidEdges( puzzle, validEdges );
    //puzzle_calculateValidEdgesStack( puzzle, validEdges );

    if ( validEdges->numElements < 4 ) {
        printf( "Error in edge solver\n" );
    }

    //for all of the valid configurations, try all possible combinations of edges
    edgeSolutions->numElements = 0;
    for ( uint i = 0; i < 6; ++i ) {
        uint edges[4];
        puzzle_recEdgeSolve( puzzle, edges, cornerArrangements[i], 
                            validEdges, 0, edgeSolutions );
    }
}

void findValidCentersForEdge( const Puzzle* const puzzle, const EdgeSolution* edgeSolution,
                              DynamicArray* centerSolutions ) {
    static const uint centerIndex[9] = { 6, 7, 8, 11, 12, 13, 16, 17, 18 };
    static bool allocatedCenters = false;
    static DynamicArray* validCenterRows;
    if ( !allocatedCenters ) {
        validCenterRows = da_create( 4000, sizeof( TripleIndex ) );
        allocatedCenters = true;
    }
    validCenterRows->numElements = 0;
    uint validNeighbors[9][9];
    uint validNeighborsCount[9];
    memset( validNeighborsCount, 0, sizeof( uint ) * 9 );
    for ( uint i = 0; i < 9; ++i ) {
        for ( uint j = 0; j < 9; ++j ) {
            if ( i == j ) {
                continue;
            }
            if ( piece_canBeNeighbors( puzzle->pieces[centerIndex[i]], puzzle->pieces[centerIndex[j]] ) ){
                validNeighbors[i][validNeighborsCount[i]] = j;
                ++validNeighborsCount[i];
            }
        }
    }

    puzzle_calculateValidCenterRows( puzzle, edgeSolution, validCenterRows,
                                     validNeighbors, validNeighborsCount );
    uint centerIndexes[3];
    puzzle_recCenterSolve( puzzle, centerIndexes, edgeSolution, validCenterRows,
                           0, centerSolutions );
}

void puzzle_findValidSolutions( const Puzzle* const puzzle,
                               PuzzleSolution* const otherSolutions,
                               uint* const numOtherSolutions, const uint maxOtherSolutions,
                               uint* const maxUniqueIndexes, uint* const maxUniqueSides ) {
    static DynamicArray* edgeSolutions;
    static DynamicArray* validCenterRows;
    static DynamicArray* centerSolutions;
    static bool allocatedArrays = false;
    if ( !allocatedArrays ) {
        //terrible idea, no real way to free this after
        centerSolutions = da_create( 2000, sizeof( CenterSolution ) );
        edgeSolutions = da_create( 1000000, sizeof( EdgeSolution ) );
        validCenterRows = da_create( 2000, sizeof( TripleIndex ) );
        allocatedArrays = true;
    }

    edgeSolutions->numElements = 0;
    centerSolutions->numElements = 0;

    puzzle_findValidEdges( puzzle, edgeSolutions );

    *maxUniqueIndexes = 0;
    *maxUniqueSides = 0;
    for ( uint i = 0; i < edgeSolutions->numElements; ++i ) {
        uint temp = centerSolutions->numElements;
        findValidCentersForEdge( puzzle, ( EdgeSolution* ) da_getElement( edgeSolutions, i ),
                                 centerSolutions );
        for  ( uint j = temp; j < centerSolutions->numElements; ++j ) {
            PuzzleSolution solution;
            puzzle_convertEdgeCenterToSolution( &solution,
                                                ( EdgeSolution* ) da_getElement( edgeSolutions, i ),
                                                ( CenterSolution* ) da_getElement( centerSolutions, j ) );
            uint numIndexConnections = 0;
            uint numSideConnections = 0;
            puzzle_calculateOriginalConnections( puzzle, &solution,
                                                &numIndexConnections,
                                                &numSideConnections );
            if ( numIndexConnections < 40 ) {
                otherSolutions[*numOtherSolutions] = solution;
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
            if ( *numOtherSolutions > 1 ) {
                return;
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

static void puzzle_setPieces2( Puzzle* const puzzle ) {
    for ( uint i = 0; i < 25; ++i ) {
        if ( i == 0 ) {
            puzzle->pieces[0] = piece_create( CORNER, i, 0, puzzle->connections[0],
                                             0, puzzle->connections[20] );
        } else if ( i == 4 ) {
            puzzle->pieces[4] = piece_create( CORNER, i, 0, puzzle->connections[24],
                                             0, puzzle->connections[15] );
        } else if ( i == 20 ) {
            puzzle->pieces[20] = piece_create( CORNER, i, 0, puzzle->connections[35],
                                              0, puzzle->connections[4] );
        } else if ( i == 24 ) {
            puzzle->pieces[24] = piece_create( CORNER, i, 0, puzzle->connections[19],
                                              0, puzzle->connections[39] );
        } else if ( i < 4 ) { //top edge
            puzzle->pieces[i] = piece_create( EDGE, i, 0, puzzle->connections[i * 5],
                                             puzzle->connections[i + 20],
                                             puzzle->connections[( i - 1 ) * 5] );
        } else if ( i % 5 == 0 ) { //left edge
            puzzle->pieces[i] = piece_create( EDGE, i, 0, puzzle->connections[i + 15],
                                             puzzle->connections[i / 5],
                                             puzzle->connections[i + 20] );
        } else if ( ( i - 4) % 5 == 0 ) { //right edge
            puzzle->pieces[i] = piece_create( EDGE, i, 0, puzzle->connections[i + 20],
                                             puzzle->connections[( i + 1 ) / 5 + 14],
                                             puzzle->connections[i + 15] );
        } else if ( i > 20 && i < 24 ) { //bottom edge
            puzzle->pieces[i] = piece_create( EDGE, i, 0, puzzle->connections[( i - 21 ) * 5 + 4],
                                             puzzle->connections[i + 15],
                                             puzzle->connections[( i - 20 ) * 5 + 4] );
        } else {
            uint row = i / 5;
            uint col = i % 5;
            puzzle->pieces[i] = piece_create( CENTER, i, puzzle->connections[i + 15],
                                             puzzle->connections[5 * col + row],
                                             puzzle->connections[i + 20],
                                             puzzle->connections[5 * ( col - 1 ) + row] );
        }
    }
}

void puzzle_mutateCenter( Puzzle* const destPuzzle, const Puzzle* const srcPuzzle, const uint minMutations, const uint maxMutations ) {
    const static uint validCenterConnections[24] = { 21, 22, 23, 1, 6, 11, 16, 2, 7, 12, 17,
                                                   3, 8, 13, 18, 26, 27, 28, 31, 32, 33,
                                                   36, 37, 38 };
    memcpy( destPuzzle, srcPuzzle, sizeof( Puzzle ) );
    uint numMutations = rand_intBetween( minMutations, maxMutations + 1 );
    while ( numMutations ) {
        int firstIndex = 0;
        int secondIndex = 0;
        while ( firstIndex == secondIndex ) {
            firstIndex = validCenterConnections[rand_index( 24 )];
            secondIndex = validCenterConnections[rand_index( 24 )];
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
    puzzle_setPieces2( destPuzzle );
}

void puzzle_shuffle( Puzzle* const puzzle ) {
    const uint numUniqueConnectors = puzzle->numUniqueConnectors;
    //const uint numEach = 40 / numUniqueConnectors; 
    const uint numEach = 2; //need to be at least 2 of each so they can swap with each other
    const int numLeftOver = 40 - numUniqueConnectors * numEach; 
    if ( numLeftOver < 0 ) {
        fprintf( stderr, "Cannot create a puzzle with %i unique connectors (will not have multiple solutions)\n", numUniqueConnectors );
        fprintf( stderr, "Max number of unique connections: %i\n", numUniqueConnectors / 2 );
        exit( 1 );
    }

    for ( uint i = 0; i < numUniqueConnectors; ++i ) {
        for ( uint j = 0; j < numEach; ++j ) {
            puzzle->connections[i * numEach + j] = i + 1;            
            if ( rand_float() < 0.5 ) {
                //puzzle->connections[i * numEach + j] *= -1;
            }
        }
    }

    for ( uint i = 0; i < numLeftOver; ++i ) {
        puzzle->connections[40  - numLeftOver + i] = rand_intBetween( 1, numUniqueConnectors + 1 );
        if ( rand_float() < 0.5 ) {
            //puzzle->connections[40 - numLeftOver + i] *= -1;
        }
    }

    rand_shuffle( puzzle->connections, 40, sizeof( char ) );

    puzzle_setPieces2( puzzle );
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
        int firstIndex = 0;
        int secondIndex = 0;
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
    puzzle_setPieces2( destPuzzle );
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

    uint bestComparison = 0;
    bool foundBestSides = false;
    for ( uint i = 0; i < numGenerations; ++i ) {
        //printf( "Starting Generation: %u/%u\n", i + 1, numGenerations );
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

void puzzle_findValidSolutions2( const Puzzle* const puzzle, DynamicArray* const edgeSolutions,
                               PuzzleSolution* const otherSolutions,
                               uint* const numOtherSolutions, const uint maxOtherSolutions,
                               uint* const maxUniqueIndexes, uint* const maxUniqueSides ) {
    static DynamicArray* centerSolutions;
    static bool allocatedArrays = false;
    if ( !allocatedArrays ) {
        //terrible idea, no real way to free this after
        centerSolutions = da_create( 2000, sizeof( CenterSolution ) );
        allocatedArrays = true;
    }

    centerSolutions->numElements = 0;

    *maxUniqueIndexes = 0;
    *maxUniqueSides = 0;
    for ( uint i = 0; i < edgeSolutions->numElements; ++i ) {
        uint temp = centerSolutions->numElements;
        findValidCentersForEdge( puzzle, ( EdgeSolution* ) da_getElement( edgeSolutions, i ),
                                 centerSolutions );
        for  ( uint j = temp; j < centerSolutions->numElements; ++j ) {
            PuzzleSolution solution;
            puzzle_convertEdgeCenterToSolution( &solution,
                                                ( EdgeSolution* ) da_getElement( edgeSolutions, i ),
                                                ( CenterSolution* ) da_getElement( centerSolutions, j ) );
            uint numIndexConnections = 0;
            uint numSideConnections = 0;
            puzzle_calculateOriginalConnections( puzzle, &solution,
                                                &numIndexConnections,
                                                &numSideConnections );
            if ( numIndexConnections < 40 ) {
                otherSolutions[*numOtherSolutions] = solution;
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
            if ( *numOtherSolutions > 1 ) {
                return;
            }
        }
    }
}

void puzzle_shuffleUntilUniqueEdge( Puzzle* const puzzle, DynamicArray* edgeSolutions ) {
    while ( true ) {
        puzzle_shuffle( puzzle );

        edgeSolutions->numElements = 0;

        puzzle_findValidEdges(puzzle, edgeSolutions);

        bool valid = false;
        for ( uint i = 0; i < edgeSolutions->numElements; ++i ) {
            if ( edgeSolutionIsUnique( ( EdgeSolution* ) da_getElement( edgeSolutions, i )) ) {
                valid = true;
                break;
            }
        }
        if ( valid ) {
            return;
        }
    }
}

void puzzle_shuffleCenter( Puzzle* const puzzle ) {
    const static uint validCenterConnections[24] = { 21, 22, 23, 1, 6, 11, 16, 2, 7, 12, 17,
                                                   3, 8, 13, 18, 26, 27, 28, 31, 32, 33,
                                                   36, 37, 38 };
    const static uint validEdgeConnections[16] = { 0, 5, 10, 15, 20, 25, 30, 35, 4, 9, 14, 19,
                                                   24, 29, 34, 39 };

    int connectionCounts[puzzle->numUniqueConnectors + 1];
    for ( uint i = 1; i < puzzle->numUniqueConnectors + 1; ++i ) {
        connectionCounts[i] = 0;
    }
    for ( uint i = 0; i < 16; ++i ) {
        ++connectionCounts[puzzle->connections[validEdgeConnections[i]]];
    }

    uint tempCenterConnections[24];
    uint numAdded = 0;
    for ( uint i = 1; i < puzzle->numUniqueConnectors + 1; ++i ) {
        for ( int j = 0; j < ( 2 - connectionCounts[i] ); ++j ) {
            tempCenterConnections[numAdded] = i;
            ++numAdded;
        }
    }

    for ( uint i = numAdded; i < 24; ++i ) {
        tempCenterConnections[i] = rand_intBetween( 1, puzzle->numUniqueConnectors + 1 );
    }

    rand_shuffle( tempCenterConnections, 24, sizeof( int ) );
    for ( uint i = 0; i < 24; ++i ) {
        puzzle->connections[validCenterConnections[i]] = tempCenterConnections[i];
    }
    
    puzzle_setPieces2( puzzle );
}

void puzzle_findSolutionsUniqueEdges() {
    Puzzle* puzzle = puzzle_create( 7 );
    DynamicArray* edgeSolutions = da_create( 10000, sizeof( EdgeSolution ) );
    puzzle_shuffleUntilUniqueEdge( puzzle, edgeSolutions );
    Puzzle* temp = malloc( sizeof( Puzzle ) );

    uint count = 0;
    uint bestSum = 0;
    uint bestSides = 0;
    uint bestIndexes = 0;
    bool foundBest = false;
    while ( true ) {
        uint maxUniqueIndexes = 0;
        uint maxUniqueSides = 0;
        uint maxOtherSolutions = 100;
        uint numOtherSolutions = 0;
        PuzzleSolution solutions[maxOtherSolutions];
        puzzle_findValidSolutions2( puzzle, edgeSolutions, solutions,
                                  &numOtherSolutions, maxOtherSolutions,
                                  &maxUniqueIndexes, &maxUniqueSides );
        if ( numOtherSolutions == 1 ) {
            uint sum = maxUniqueSides + maxUniqueIndexes;
            if ( sum > bestSum ) {
                foundBest = true;
                count = 0;
                bestSum = sum;
                printf( "Found puzzle with only 1 other solution!\n" );
                puzzle_printSolution( &solutions[0] );
                printf( "%i: %i sides + %i indexes\n", sum, maxUniqueSides, maxUniqueIndexes );
                for ( uint j = 0; j < 40; ++j ) {
                    if ( j ) {
                        printf( ", " );
                    }
                    printf( "%i", puzzle->connections[j] );
                }
                printf( "\n" );
            }
            
        }
        if ( foundBest ) {
            puzzle_mutateCenter( temp, puzzle, 1, 6 );
            memcpy( puzzle, temp, sizeof( Puzzle ) );
        } else {
            puzzle_shuffleCenter( puzzle );
        }
        //memcpy( puzzle, temp, sizeof( Puzzle ) );
        ++count;
        if ( count == 1000 ) {
            foundBest = false;
            count = 0;
            puzzle_shuffleUntilUniqueEdge( puzzle, edgeSolutions );
        }
    }
}
