/*This module covers the code representation of the bytecode in our VM*/

#ifndef cpandi_chunk_h
#define cpandi_chunk_h

#include "common.h"

/*We create a typedef called enum which
Stores the operation codes, these assist us in developing the byte code
and are called op codes for short*/

typedef enum {
OP_RETURN,
} OpCode;

/*This struct is a dynamic array which stores the count and the capacity*/
typedef struct {
    int count;
    int capacity;
    /*And a code !!!*/
    uint8_t* code;
} Chunk;


/*The init chunk is a method to initialize the Chunk struct*/
void initChunk(Chunk* chunk);

/*The method helps free the memory as required after the initialisation*/
void freeChunk(Chunk* chunk);

/*This method is used for appending a byte to the end of the chunk*/
void writeChunk(Chunk* chunk, uint8_t byte);


#endif