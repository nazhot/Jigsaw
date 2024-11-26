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
    uint32_t sides; //left, bottom, right, top
    uint32_t inverseSides; //right, top, left, bottom
    char index;
} Piece;


Piece piece_create( const PieceType type, const uint index );
char piece_getSide( const Piece piece, const SideDirection side );
char piece_getInverseSide( const Piece piece, const SideDirection side );
char piece_getSideWithRotation( const Piece piece, const SideDirection side,
                                const uint rotation );
char piece_getInverseSideWithRotation( const Piece piece, const SideDirection side,
                                const uint rotation );
void piece_setSide( Piece* const piece, const SideDirection side, const unsigned char value );
void piece_printSides( const uint32_t sides );
void piece_print( const Piece* piece );

#endif
