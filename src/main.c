#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "puzzle.h"
#include "pieces.h"

int main( int argc, char *argv[] ) {

    const uint numUniqueConnections = 7;
    const uint generationSize = 10000;
    const uint numGenerations = 100000;
    const uint numSurivors = 40;
    const uint numChildren = 50;
    const uint minMutations = 1;
    const uint maxMutations = 10;

    puzzle_findMostUniqueSolution( numUniqueConnections, generationSize, numGenerations,
                                   numSurivors, numChildren, minMutations, maxMutations );

    return 0;
}
