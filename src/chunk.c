#include <stdlib.h>
#include "memory.h"
#include "chunk.h"

void initChunk(Chunk* chunk) {
    /*The count is initially set to 0*/
    chunk->count = 0;
    /*The capacity is initially set to 0*/
    chunk->capacity = 0;
    //This is the array that will store the chunks of code
    chunk->code = NULL;
}

void freeChunk(Chunk* chunk) {
    //The array is first freed using the FREE_ARRAY macro
    FREE_ARRAY(uint8_t, chunk->code, chunk->capacity);
    //The next step after completely cleaning the array is that we call the init_chunk to 
    // return the array to an empty state :)
    initChunk(chunk);
}




void writeChunk(Chunk* chunk, uint8_t byte) {
    //First check if the capacity of the chunk is lower than the count + 1
    if (chunk->capacity < chunk->count + 1) {
        //If the capacity is low, then we grow the capacity
        int oldCapacity = chunk->capacity;
        chunk->capacity = GROW_CAPACITY(oldCapacity);
        //And we grow the array
        chunk->code = GROW_ARRAY(uint8_t, chunk->code, oldCapacity, chunk->capacity);
    }
    //Next we go into the chunk's array and store the code in the last byte (count)
    chunk->code[chunk->count] = byte;
    //Increment the count to the next byte !!
    chunk->count++;
}

