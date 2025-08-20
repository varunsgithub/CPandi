/*This module covers the code representation of the bytecode in our VM*/

#ifndef cpandi_chunk_h
#define cpandi_chunk_h

#include "common.h"
#include "value.h"

/*We create a typedef called enum which
Stores the operation codes, these assist us in developing the byte code
and are called op codes for short*/

typedef enum {
OP_CONSTANT,
OP_NIL,
OP_TRUE,
OP_FALSE,
OP_POP,
OP_GET_LOCAL,
OP_SET_LOCAL,
OP_GET_GLOBAL,
OP_DEFINE_GLOBAL,
OP_SET_GLOBAL,
OP_EQUAL,
OP_GREATER,
OP_LESS,
OP_ADD,
OP_SUBTRACT,
OP_MULTIPLY,
OP_DIVIDE,
OP_NOT,
OP_NEGATE,
OP_PRINT,
OP_JUMP,
OP_JUMP_IF_FALSE,
OP_RETURN,
} OpCode;

/*This struct is a dynamic array which stores the count and the capacity*/
typedef struct {
    int count;
    int capacity;
    /*And an array for bytecode !!!*/
    uint8_t* code;
    //for storing the line numbers
    int* lines;
    ValueArray constants;
} Chunk;


/*The init chunk is a method to initialize the Chunk struct*/
void initChunk(Chunk* chunk);

/*The method helps free the memory as required after the initialisation*/
void freeChunk(Chunk* chunk);

/*This method is used for appending a byte to the end of the chunk*/
void writeChunk(Chunk* chunk, uint8_t byte, int line);

/*Method to add constants to the chunk*/
int addConstant(Chunk* chunk, Value value);

#endif