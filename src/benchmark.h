#ifndef BENCHMARK_H
#define BENCHMARK_H

#include <stdlib.h>

void generateSwappablePuzzle( const uint numUniqueConnections );
void benchmark_puzzleSolve( const uint numPuzzles, const char* const description );

#endif
