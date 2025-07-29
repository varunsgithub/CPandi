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
    //The line number is set to null in beg.
    chunk->lines = NULL;
    //Initialising the constants !!
    initValueArray(&chunk->constants);
}

void freeChunk(Chunk* chunk) {
    //The array is first freed using the FREE_ARRAY macro
    FREE_ARRAY(uint8_t, chunk->code, chunk->capacity);
    //Free the line number array
    FREE_ARRAY(int, chunk->lines, chunk->capacity);
    freeValueArray(&chunk->constants);
    //The next step after completely cleaning the array is that we call the init_chunk to 
    // return the array to an empty state :)
    initChunk(chunk);
}




void writeChunk(Chunk* chunk, uint8_t byte, int line) {
    //First check if the capacity of the chunk is lower than the count + 1
    if (chunk->capacity < chunk->count + 1) {
        //If the capacity is low, then we grow the capacity
        int oldCapacity = chunk->capacity;
        chunk->capacity = GROW_CAPACITY(oldCapacity);
        //And we grow the array
        chunk->code = GROW_ARRAY(uint8_t, chunk->code, oldCapacity, chunk->capacity);
        //and we also grow the line array
        chunk->lines = GROW_ARRAY(int, chunk->lines, oldCapacity, chunk->capacity);
    }
    //Next we go into the chunk's array and store the code in the last byte (count)
    chunk->code[chunk->count] = byte;
    //store the line numbers in the line array
    chunk->lines[chunk->count] = line;
    //Increment the count to the next byte !!
    chunk->count++;
}

/*This method writes the values to the chunk and then returns the 0 based index 
of the last element appended*/
int addConstant(Chunk* chunk, Value value) {
    writeValueArray(&chunk->constants, value);
    return chunk->constants.count - 1;
}