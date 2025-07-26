#include "common.h"
#include "chunk.h"
#include "debug.h"

int main (int argc, const char* argv[]) {
    //Create a struct chunk
    Chunk chunk;
    //Initialize it
    initChunk(&chunk);
    
    
    //Test
    //Storing the constant in the constant pool
    int constant = addConstant(&chunk, 1.2);
    //Write code to the main chunk with OP CODE and the Constant
    writeChunk(&chunk, OP_CONSTANT, 123);
    writeChunk(&chunk, constant, 123);
    


    //Write it ?
    writeChunk(&chunk, OP_RETURN, 123);
    
    //Trying out the tests
    disassembleChunk(&chunk, "test_chunk");

    //free it !!
    freeChunk(&chunk);
    return 0;
}
