#include "common.h"
#include "chunk.h"

int main (int argc, const char* argv[]) {
    //Create a struct chunk
    Chunk chunk;
    //Initialize it
    initChunk(&chunk);
    //Write it ?
    writeChunk(&chunk, OP_RETURN);
    //free it !!
    freeChunk(&chunk);
    return 0;
}
