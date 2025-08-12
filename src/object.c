#include <stdio.h>
#include <string.h>

#include "memory.h"
#include "object.h"
#include "value.h"
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

/*The allocate string function first creates an Obj pointer, then it stores the length,
chars and returns the ObjString* */
static ObjString* allocateString(char* chars, int length) {
    ObjString* string = ALLOCATE_OBJ(ObjString, OBJ_STRING);
    string->length = length;
    string->chars = chars;
    return string;
}

ObjString* takeString(char* chars, int length) {
  return allocateString(chars, length);
}

ObjString* copyString(const char* chars, int length) {
    //Create a char array to help allocate the char with a size of length + 1 (To accomodate the \0)
    char* heapChars = ALLOCATE(char, length + 1);
    //copy the chars into the heap array
    memcpy(heapChars, chars, length);
    //last element is the null termination
    heapChars[length] = '\0';
    return allocateString(heapChars, length);
}

void printObject(Value value) {
    switch (OBJ_TYPE(value)) {
        case OBJ_STRING:
            printf("%s", AS_CSTRING(value));
            break;
    }
}