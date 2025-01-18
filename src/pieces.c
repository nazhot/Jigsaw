#include "pieces.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <inttypes.h>


__inline__ Piece piece_create( const PieceType type, const uint index, const char top,
                               const char right, const char bottom, const char left ) {
    Piece piece = ( Piece ) { .sides = { [TOP] = top, [RIGHT] = right, [BOTTOM] = bottom,
                                  [LEFT] = left },
                              .type = type,
                              .index = index,
                              .bitfield = 0 };

    piece.bitfield |= ( ( uint64_t ) 1 ) << top;
    piece.bitfield |= ( ( uint64_t ) 1 ) << right;
    piece.bitfield |= ( ( uint64_t ) 1 ) << bottom;
    piece.bitfield |= ( ( uint64_t ) 1 ) << left;
    return piece;
}

__inline__ bool piece_contains( const Piece piece, const char side ) {
    return piece.bitfield >> side & 1;
}

__inline__ char piece_getSide( const Piece piece, const SideDirection side ) {
    return piece.sides[side];
}

__inline__ char piece_getSideWithRotation( const Piece piece, const SideDirection side,
                                const uint rotation ) {
    return piece.sides[( side + rotation ) % 4 ];
}

__inline__ bool piece_piecesConnect( const char side1, const char side2 ) {
    //return side1 + side2 == 0;
    return side1 == side2;
}

void piece_print( const Piece* const piece ) {
    const static char* types[] = { [CENTER] = "Center", [CORNER] = "Corner",
        [EDGE] = "Edge" };
    printf( "%i: %i, %i, %i, %i (%s)\n", piece->index, piece_getSide( *piece, LEFT ),
           piece_getSide( *piece, BOTTOM ),
           piece_getSide( *piece, RIGHT ),
           piece_getSide( *piece, TOP ),
           types[piece->type] );
}
