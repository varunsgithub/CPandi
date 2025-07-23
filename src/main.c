#include "common.h"
#include "chunk.h"
#include "debug.h"

int main (int argc, const char* argv[]) {
    //Create a struct chunk
    Chunk chunk;
    //Initialize it
    initChunk(&chunk);
    //Write it ?
    writeChunk(&chunk, OP_RETURN);
    
    //Trying out the tests
    disassembleChunk(&chunk, "test_chunk");

    //free it !!
    freeChunk(&chunk);
    return 0;
}
