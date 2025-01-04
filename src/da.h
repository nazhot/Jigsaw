#ifndef DA_H
#define DA_H

#include <stdlib.h>
#include <stdbool.h>

typedef struct DynamicArray {
    void* contents;
    size_t elementSize;
    size_t size;
    size_t numElements;
    unsigned int multFactor; /**When resizing, multiply current size by this value*/
    unsigned int addFactor; /**When resizing, add this value to the current size*/
} DynamicArray;

/**
 * Creates a new DynamicArray
 *
 * Default values:
 *  - multFactor: 2
 *  - addFactor: 0
 */
DynamicArray* da_create( size_t startingSize, size_t elementSize );

/**
 * Append element to end of DynamicArray
 *
 * DynamicArray will automatically resize if it is full
 */
void da_addElement( DynamicArray* da, void* element );

bool da_addElementIfAble( DynamicArray* da, void* element );

/**
 * Get element at specified index
 */
void* da_getElement( const DynamicArray* const da, const unsigned int index );

/**
 * Replace the element at index with the provided element
 */
void da_replaceElement( DynamicArray* da, void* element, unsigned int index );


/**
 * Free DynamicArray
 */
void da_free( DynamicArray* da );


#endif
