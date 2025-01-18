#ifndef PIECES_H
#define PIECES_H

#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>


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

//abs( sides[x] ) is [0, numUniqueConnections)
//negative is an innie, positive it outie
//Pieces had previously held their own rotation, but only 9 Pieces have rotation
typedef struct Piece {
    PieceType type;
    char sides[4]; //top, right, bottom, left
    char index; //index of Piece within the original Puzzle (0 - 25, left to right
                //top to bottom)
    uint64_t bitfield;
} Piece;

bool piece_contains( const Piece piece, const char side );

/*
 * Create a Piece with its type and sides
 *
 * Pieces really only need to be created once, which is when the Puzzle is created,
 * or when a shuffle is being done. After the Puzzle has been set, this is never
 * called again
*/
Piece piece_create( const PieceType type, const uint index, const char top,
                    const char right, const char bottom, const char left );

/*
 * Get a side from a Piece, without any kind of rotation
 *
 * Side just points to an element in the sides array, if rotation of the piece
 * is needed use piece_getSideWithRotation
*/
char piece_getSide( const Piece piece, const SideDirection side );

/*
 * Get a side from a Piece, using rotation
 *
 * Rotation is [0,3]. It can technically be any positive value, the index is
 * modulo'd by 4 before grabbing the element from sides. 
 * Rotation of 0 is the Piece as normal, every increment is
 * rotating the Piece counter-clockwise. E.g. Trying to get Top with a rotation
 * of 1 will retrieve the Right side of the Piece
*/
char piece_getSideWithRotation( const Piece piece, const SideDirection side,
                                const uint rotation );

/*
 * Print the Piece, no rotation
*/

bool piece_piecesConnect( const char side1, const char side2 );
void piece_print( const Piece* piece );

#endif
