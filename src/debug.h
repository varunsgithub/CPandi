#ifndef cpandi_debug_h
#define cpandi_debug_h

#include "chunk.h"

/*This method helps with disassembling the given instructions */
void disassemble(Chunk* chunk, const char* name);

int disassembleInstruction(Chunk* chunk, int offset);

#endif