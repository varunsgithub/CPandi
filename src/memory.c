#include <stdlib.h>

#include "memory.h"
#include "vm.h"

void* reallocate(void* pointer, size_t oldSize, size_t newSize) {
    if (newSize == 0) {
        //Free the pointers if the new size is to be 0
        free(pointer);
        //return NULL
        return NULL;
    }

    //The re alloc function actually helps resize the array to new size
    // if the old size is 0, it automatically callls malloc :)
    void* result = realloc(pointer, newSize);
    
    if (result == NULL) {
        //If the realloc function fails to produce any memory
        // then we exit with system code 1 (exit failure)
        exit(1);
    }

    return result;
}

static void freeObject(Obj* object) {
    switch (object->type) {
        //Cast the object to the correct type
        case OBJ_STRING: {
            ObjString* string = (ObjString*)object;
            //Then clean the char array for a string including the null terminator (+1)
            FREE_ARRAY(char, string->chars, string->length+1);
            //free the memory allocated for the pointer
            FREE(ObjString, object);
            break;
        }
    }
}

void freeObjects() {
    Obj* object = vm.objects;
    while (object != NULL) {
        Obj* next = object->next;
        freeObject(object);
        object = next;
    }
}