#include "da.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

DynamicArray* da_create( size_t startingSize, size_t elementSize ) {
    DynamicArray *da = malloc( sizeof( DynamicArray ) );
    if ( !da ) {
        fprintf( stderr, "Couldn't create DynamicArray\n" );
        exit( 1 );
    }
    da->elementSize = elementSize;
    da->size = startingSize;
    da->numElements = 0;
    da->addFactor = 0;
    da->multFactor = 2;
    da->contents = malloc( startingSize * elementSize );
    if ( !da->contents ) {
        fprintf( stderr, "Could not create %lu elements of size %lu\n", startingSize, elementSize );
        exit( 1 );
    }

    return da;
}

static void da_resizeBasedOnFactor( DynamicArray* da ) {
    da->size *= da->multFactor;
    da->size += da->addFactor;
    da->contents = realloc( da->contents, da->size * da->elementSize );
    if ( !da->contents ) {
        fprintf( stderr, "Could not resize to %lu elements of size %lu\n", da->size, da->elementSize );
        exit( 1 );
    }
}

void da_addElement( DynamicArray* da, void* element ) {
    if ( !da || !da->contents ) {
        return;
    }
    if ( da->numElements == da->size ) {
        da_resizeBasedOnFactor( da );
    }
    memcpy( &da->contents[da->elementSize * da->numElements++], element, da->elementSize );
}

bool da_addElementIfAble( DynamicArray* da, void* element ) {
    if ( !da || !da->contents || da->numElements == da->size ) {
        return false;
    }
    memcpy( &da->contents[da->elementSize * da->numElements++], element, da->elementSize );
    return true;
}

void* da_getElement( const DynamicArray* const da, const unsigned int index ) {
    if ( !da || index >= da->numElements ) {
        return NULL;
    }
    return &da->contents[index * da->elementSize];
}

void da_replaceElement( DynamicArray* da, void* element, unsigned int index ) {
    if ( !da || index >= da->numElements ) {
        return;
    }
    memcpy( da_getElement( da, index ), element, da->elementSize );
}

void da_free( DynamicArray *da ) {
    free( da->contents );
    free( da );
}
