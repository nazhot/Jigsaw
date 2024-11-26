#ifndef PUZZLE_H
#define PUZZLE_H

#include <stdbool.h>
#include <stdlib.h>
#include "pieces.h"

#define OUTIE_BITMASK 128


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

typedef struct Puzzle {
    Piece pieces[25];
    Piece* edgePieces[12];
    Piece* cornerPieces[4];
    Piece* centerPieces[9];

    char connections[40]; //vertical connections first, top to bottom, left to right
                          //negative is innie -> outie
    uint numUniqueConnectors; 

    EdgeSolution* edgeSolutions;
    uint numEdgeSolutions;
    uint maxEdgeSolutions;

    CenterSolution* centerSolutions;
    uint numCenterSolutions;
    uint maxCenterSolutions;
} Puzzle;


void puzzle_findValidSolutions( const Puzzle* const puzzle );
void puzzle_shuffle( Puzzle* const puzzle );
Puzzle* puzzle_create( const uint numUniqueConnectors );
void puzzle_printLayout( const Puzzle* const puzzle );
void puzzle_free( Puzzle* const puzzle );

#endif
