#include "common.h"
#include "chunk.h"
#include "debug.h"
#include "vm.h"

int main (int argc, const char* argv[]) {
    //Initialize a VM when the program runs
    initVM();
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

      constant = addConstant(&chunk, 3.4);
  writeChunk(&chunk, OP_CONSTANT, 123);
  writeChunk(&chunk, constant, 123);

  writeChunk(&chunk, OP_ADD, 123);

  constant = addConstant(&chunk, 5.6);
  writeChunk(&chunk, OP_CONSTANT, 123);
  writeChunk(&chunk, constant, 123);

  writeChunk(&chunk, OP_DIVIDE, 123);



    writeChunk(&chunk, OP_NEGATE, 123);


    //Write it ?
    writeChunk(&chunk, OP_RETURN, 123);
    
    //Trying out the tests
    disassembleChunk(&chunk, "test_chunk");
    //Interpret a chunk of bytecode and lo ! the VM is here on its duty
    interpret(&chunk);
    //Free the VM when exiting
    freeVM();
    //free it !!
    freeChunk(&chunk);
    return 0;
}
