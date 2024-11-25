#ifndef PUZZLE_H
#define PUZZLE_H

#include <stdbool.h>
#include <stdlib.h>
#include "pieces.h"

#define OUTIE_BITMASK 128


typedef struct PuzzleSolution {
    char indexes[25];
    char rotations[25];
} PuzzleSolution;

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


PuzzleSolution* puzzle_findValidSolutions( const Puzzle* const puzzle );
void puzzle_shuffle( Puzzle* const puzzle );
Puzzle* puzzle_create( const uint numUniqueConnectors );
void puzzle_printLayout( const Puzzle* const puzzle );

#endif
