#ifndef cpandi_object_h
#define cpandi_object_h

#include "common.h"
#include "value.h"
#include "chunk.h"

/*These macro fetches the type identifier*/
#define OBJ_TYPE(value)     (AS_OBJ(value)->type)

#define IS_FUNCTION(value)  isObjType(value, OBJ_FUNCTION)

#define IS_NATIVE(value)       isObjType(value, OBJ_NATIVE)

/*These macros check if the given the values are of the requisite type*/
#define IS_STRING(value)    isObjType(value, OBJ_STRING)

/*These macros help in downcasting the Obj Value to a ObjString* */
#define AS_STRING(value)    ((ObjString*) AS_OBJ(value))
#define AS_CSTRING(value)   (((ObjString*) AS_OBJ(value)) -> chars)


/*These macros help in downcasting the Obj pointers to a Function object*/
#define AS_FUNCTION(value)  ((ObjFunction*)AS_OBJ(value))

#define AS_NATIVE(value) \
    (((ObjNative*)AS_OBJ(value))->function)

/*These are the identifiers type which help identify the Object*/
typedef enum {
    OBJ_FUNCTION,
    OBJ_NATIVE,
    OBJ_STRING,
} ObjType;

struct Obj {
    //The first tag is an identifier for storing the size !
    ObjType type;
    //the second tag helps in identifying memory objects
    //This is done by creating a linked list like structure which holds the reference to the next object
    struct Obj* next;
};


typedef struct {
    Obj obj;
    int arity;
    Chunk chunk;
    ObjString* name;
} ObjFunction;

typedef Value (*NativeFn)(int argCount, Value* args);

typedef struct {
    Obj obj;
    NativeFn function;
} ObjNative;

struct ObjString {
    //The obj pointer stores the type !
    Obj obj;
    int length;
    char* chars;
    uint32_t hash;
};

/*This method initiallizes a new function object*/
ObjFunction* newFunction();

/*This method is a constructor for the native functions*/
ObjNative* newNative(NativeFn function);

ObjString* takeString(char* chars, int length);

/*This method helps the compiler emit the string bytecode !*/
ObjString* copyString(const char* chars, int length);

/*This method helps print the strings in debugging mode*/
void printObject(Value value);

/*This function checks if the given value is an object of the specified type*/
static inline bool isObjType(Value value, ObjType type) {
    return IS_OBJ(value) && AS_OBJ(value)->type == type;
}

#endif