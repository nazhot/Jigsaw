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
    uint index;
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

typedef struct EdgePiece {
    unsigned char index;
    uint possibleLeftPieces[11];
    uint possibleRightPieces[11];

    uint leftConnector: 6;
    uint rightConnector: 6;
    uint innerConnector: 6;

    uint leftConnectorIsInnie: 1;
    uint rightConnectorIsInnie: 1;
    uint innerConnectorIsInnie: 1;
} EdgePiece;


typedef struct CornerPiece {
    unsigned char index;
    uint possibleBottomPieces[11];
    uint possibleRightPieces[11];

    uint bottomConnector: 6;
    uint rightConnector: 6;

    uint bottomConnectorIsInnie: 1;
    uint rightConnectorIsInnie: 1;
} CornerPiece;

typedef struct CenterPiece {
    unsigned char index;
    uint possibleLeftPieces[21];
    uint possibleRightPieces[21];
    uint possibleTopPieces[21];

    uint leftConnector: 6;
    uint rightConnector: 6;
    uint topConnecter: 6;
    uint bottomConnector: 6;

    uint leftConnectorIsInnie: 1;
    uint rightConnectorIsInnie: 1;
    uint topConnectorIsInnie: 1;
    uint bottomConnectorIsInnie: 1;
} CenterPiece;

#endif
