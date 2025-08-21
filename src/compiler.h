#ifndef cpandi_compiler_h
#define cpandi_compiler_h

#include "object.h"
#include "vm.h"

ObjFunction* compile(const char* source);

#endif