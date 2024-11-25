#ifndef PUZZLE_H
#define PUZZLE_H

#include <stdbool.h>
#include <stdlib.h>
#include "pieces.h"

#define OUTIE_BITMASK 128


typedef struct Puzzle {
    Piece pieces[25];
    char pieceIndexes[25];
    uint rotations[25];
    Piece* edgePieces[12];
    Piece* cornerPieces[4];
    Piece* centerPieces[9];

    char connections[40]; //vertical connections first, top to bottom, left to right
                          //negative is innie -> outie
    uint numUniqueConnectors; 
} Puzzle;


int puzzle_solveEdges( const Puzzle* const puzzle );
void puzzle_shuffle( Puzzle* const puzzle );
Puzzle* puzzle_create( const uint numUniqueConnectors );
void puzzle_printLayout( const Puzzle* const puzzle );

#endif
