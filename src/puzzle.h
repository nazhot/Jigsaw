#ifndef PUZZLE_H
#define PUZZLE_H

#include <stdbool.h>
#include <stdlib.h>
#include "pieces.h"

typedef struct PuzzleSolution {
    char indexes[25];
    char rotations[25];
} PuzzleSolution;

/*
 * Holds the information about a Puzzle instance. 
 *
 * The edge/corner/center arrays point to the Pieces with the pieces array, and
 * are used to easily cycle through those specific types when solving a Puzzle.
 *
 * TODO: look at removing numUniqueConnectors, only really used in the initial
 * creation
*/
typedef struct Puzzle {
    Piece pieces[25];

    char connections[40]; //[0, 19] vertical connections, top to bottom, 
                          //[20, 39] horizontal connections, left to right
                          //negative is innie -> outie
    uint numUniqueConnectors; 
} Puzzle;


/*
 * Create a Puzzle that contains numUniqueConnectors amount of different connections
 *
 * The Puzzle created will have the connections randomly distributed, Any extras
 * (40 % numUniqueConnectors != 0) are chosen randomly. All Puzzles have the solution
 * of Pieces with no rotation, going from left - right, top - bottom from 0 - 25.
 *
 * The main action is calling puzzle_shuffle after mallocing the Puzzle, futher
 * explained in that function.
*/
Puzzle* puzzle_create( const uint numUniqueConnectors );

/*
 * Change the connections between Pieces with the given Puzzle
 *
 * The way a Puzzle is generated is by creating the connections array, and setting
 * the Pieces based on it. Because of this, every Puzzle for sure has a solution, and
 * the default is indexes 0 - 25 spanning from left to right, top to bottom.
 *
 * When setting the connections, there is a 50/50 shot of them being innie - outie
 * or outie - innie. 
*/
void puzzle_shuffle( Puzzle* const puzzle );

/*
 * Print the Puzzle layout
*/
void puzzle_printLayout( const Puzzle* const puzzle );

/*
 * Find all possible solutions to the given Puzzle, return how many there are
 *
 * Heart of the program. The basic thought process is this:
 * - There are only 6 valid corner configurations for any given puzzle (removing
 *   corner configurations that are just rotations of other ones)
 *
 * - Corners are set up to all have a left/right side, no top/bottom, and edges
 *   all have left/right/bottom. The sides are set up as if the top left corner
 *   was the start of a straight line going left - right, and the corners were
 *   configured to have left/right sides
 * 
 * - Go through all corner configurations, and determine valid triples of edges
 *   that could fit between corners, and store them
 *
 * - For all of those edge triples, try to find (4) of them that work for each
 *   corner configuration, and store those valid ones as EdgeSolutions
 *
 * - For all of these valid EdgeSolutions, start finding valid tripes of center
 *   pieces, with all 4 possible rotations, and store those triples
 *
 * - For the same EdgeSolution used to check valid triples, check if (3) of them
 *   one on top of another would fit with in, store any that do as a PuzzleSolution
 *   
 * - Go through all the PuzzleSolutions and do what you want with them
*/
void puzzle_findValidSolutions( const Puzzle* const puzzle,
                                PuzzleSolution* const otherSolutions,
                                uint* const numOtherSolutions, const uint maxOtherSolutions,
                                uint* const maxUniqueIndexes, uint* const maxUniqueSides );

void puzzle_findMostUniqueSolution( const uint numUniqueConnections,
                                    const uint generationSize,
                                    const uint numGenerations,
                                    const uint numSurvivors, const uint numChildren,
                                    const uint minMutations, const uint maxMutations );

/*
 * Free the given Puzzle
*/
void puzzle_free( Puzzle* const puzzle );

#endif
