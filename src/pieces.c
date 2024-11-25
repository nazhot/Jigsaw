#include "pieces.h"
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <inttypes.h>


__inline__ Piece piece_create( const PieceType type, const uint index ) {
   return ( Piece ) { .sides = 0,
                      .inverseSides = 0,
                      .type = type,
                      .index = index };
}

__inline__ char piece_getSide( const Piece piece, const SideDirection side ) {
    return ( piece.sides >> ( ( side ) % 4 ) * 8 ) & 255;
}

__inline__ char piece_getInverseSide( const Piece piece, const SideDirection side ) {
    return ( piece.inverseSides >> ( ( side + 2 ) % 4 ) * 8 ) & 255;
}

__inline__ char piece_getSideWithRotation( const Piece piece, const SideDirection side,
                                const uint rotation ) {
    return ( piece.sides >> ( ( side + rotation ) % 4 ) * 8 ) & 255;
}

__inline__ char piece_getInverseSideWithRotation( const Piece piece, const SideDirection side,
                                const uint rotation ) {
    return ( piece.inverseSides >> ( ( side + 2 + rotation) % 4 ) * 8 ) & 255;
}

static void printChar( const char value ) {
    for ( int i = 7; i >= 0; --i ) {
        printf( "%i", value >> i & 1 ? 1 : 0 );
    }
    printf( "\n" );
}

void piece_setSide( Piece *const piece, const SideDirection side, const unsigned char value ) {
    unsigned char temp = ( unsigned char ) value;
    piece->sides |= ( temp << ( ( ( side ) % 4 ) * 8 ) );
    piece->inverseSides |= value << ( ( ( side + 2 ) % 4 ) * 8 );
}

void piece_printSides( const uint32_t sides ) {
    for ( int i = 31; i >= 0; --i ) {
        if ( i != 31 && ( i + 1 ) % 8 == 0 ) {
            printf( " " );
        }
        printf( "%i", sides >> i & 1 ? 1 : 0 );
    }
    printf( "\n" );
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
