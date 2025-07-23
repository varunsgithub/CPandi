#include <stdlib.h>

#include "memory.h"

void* reallocate(void* pointer, size_t oldSize, size_t newSize) {
    if (newSize == 0) {
        //Free the pointers if the new size is to be 0
        free(pointer);
        //return NULL
        return NULL;
    }

    //The re alloc function actually helps resize the array to new size
    // if the old size is 0, it automatically callls malloc :)
    void* result = realloc(pointer, newSize);
    
    if (result == NULL) {
        //If the realloc function fails to produce any memory
        // then we exit with system code 1 (exit failure)
        exit(1);
    }

    return result;
}