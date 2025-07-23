#include <stdio.h>

#include "debug.h"

void disassembleChunk(Chunk* chunk, const char* name) {
    //Print the beginning with the name !
    printf("== %s ==\n", name);

    //start with an offset of 0, while the offset is less than 
    //array's length, disassemble the instructions in the
    for (int offset = 0; offset < chunk->count;) {
        offset = disassembleInstruction(chunk, offset);
    }
}

int disassembleInstruction(Chunk* chunk, int offset) {
    //Print the offset !
    printf("%04d ", offset);
    
    //the instruction is then taken in from the array !
    // from the offset
    uint8_t instruction = chunk->code[offset];

    //Check the instruction
    switch(instruction) {
        //If it is OP_Return then return, Simple Instructions
        case OP_RETURN:
            return simpleInstruction("OP_RETURN", offset);
        default:
            printf("Unknown opcode %d\n", instruction);
            return offset + 1;
    }
}

static int simpleInstruction(const char* name, int offset) {
    printf("%s\n", name);
    return offset + 1;
}

