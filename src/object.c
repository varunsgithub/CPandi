#include <stdio.h>
#include <string.h>

#include "memory.h"
#include "object.h"
#include "value.h"
#include "table.h"
#include "vm.h"

/*This macro helps create an object pointer and returns the same*/
#define ALLOCATE_OBJ(type, objectType) \
    (type*)allocateObject(sizeof(type), objectType)

/*This method creates a dynamically reallocated Object pointer and 
stores the type and returns the same*/
//It uses downcasting principles as the size of the object is passed as an argument, so even though
// an Obj* is created but it can be downcasted to one of the base types of the relevant size
static Obj* allocateObject(size_t size, ObjType type) {
    Obj* object = (Obj*)reallocate(NULL, 0, size);
    object->type = type;
    
    //The next pointer stores the reference of the previous head
    object->next = vm.objects;
    //The new head is then updated to the current object
    vm.objects = object;

    return object;
}

ObjFunction* newFunction() {
    //allocate space for a new object
    ObjFunction* function = ALLOCATE_OBJ(ObjFunction, OBJ_FUNCTION);
    //set everything else to 0
    function->arity = 0;
    function->name = NULL;
    initChunk(&function->chunk);
    return function;
}

/*The allocate string function first creates an Obj pointer, then it stores the length,
chars and returns the ObjString* */
static ObjString* allocateString(char* chars, int length, uint32_t hash) {
    ObjString* string = ALLOCATE_OBJ(ObjString, OBJ_STRING);
    string->length = length;
    string->chars = chars;
    string->hash = hash;
    tableSet(&vm.strings, string, NIL_VAL);
    return string;
}

/*The hashstring function uses the FNV-1a hashing function to calculate the hash key*/
static uint32_t hashString(const char* key, int length) {
    uint32_t hash = 2166136261u;
    for (int i = 0; i < length; i++) {
        hash ^= (uint8_t) key[i];
        hash *= 16777619;
    }
    return hash;
}


ObjString* takeString(char* chars, int length) {
  uint32_t hash = hashString(chars, length);

  ObjString* interned = tableFindString(&vm.strings, chars, length, hash);

  if (interned != NULL) {
    FREE_ARRAY(char, chars, length+1);
    return interned;
  }

  return allocateString(chars, length, hash);
}

ObjString* copyString(const char* chars, int length) {
    uint32_t hash = hashString(chars, length);
    
    ObjString* interned = tableFindString(&vm.strings, chars, length, hash);
    if (interned != NULL) return interned;
    
    //Create a char array to help allocate the char with a size of length + 1 (To accomodate the \0)
    char* heapChars = ALLOCATE(char, length + 1);
    //copy the chars into the heap array
    memcpy(heapChars, chars, length);
    //last element is the null termination
    heapChars[length] = '\0';
    return allocateString(heapChars, length, hash);
}

static void printFunction(ObjFunction* function) {
    if (function->name == NULL) {
        printf("<script>");
        return;
    }
    
    printf("<fn %s>", function->name->chars);
}

void printObject(Value value) {
    switch (OBJ_TYPE(value)) {
        case OBJ_FUNCTION:
            printFunction(AS_FUNCTION(value));
            break;
        case OBJ_STRING:
            printf("%s", AS_CSTRING(value));
            break;
    }
}