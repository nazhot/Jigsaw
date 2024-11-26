#ifndef PIECES_H
#define PIECES_H

#include <stdbool.h>
#include <stdlib.h>
#include <inttypes.h>


typedef enum SideDirection {
    TOP = 0,
    RIGHT,
    BOTTOM,
    LEFT
} SideDirection;

typedef enum PieceType {
    EDGE,
    CORNER,
    CENTER
} PieceType;

//sides are stored in a byte
//abs( value ) is [0, numUniqueConnections)
//negative is an innie, positive it outie
typedef struct Piece {
    PieceType type;
    char sides[4]; //top, right, bottom, left
    char index;
} Piece;


Piece piece_create( const PieceType type, const uint index, const char top,
                    const char right, const char bottom, const char left );
char piece_getSide( const Piece piece, const SideDirection side );
char piece_getSideWithRotation( const Piece piece, const SideDirection side,
                                const uint rotation );
void piece_print( const Piece* piece );

#endif
