#ifndef cpandi_object_h
#define cpandi_object_h

#include "common.h"
#include "value.h"

/*These macro fetches the type identifier*/
#define OBJ_TYPE(value)     (AS_OBJ(value)->type)

/*These macros check if the given the values are of the requisite type*/
#define IS_STRING(value)    isObjType(value, OBJ_STRING)

/*These macros help in downcasting the Obj Value to a ObjString* */
#define AS_STRING(value)    ((ObjString*) AS_OBJ(value))
#define AS_CSTRING(value)   (((ObjString*) AS_OBJ(value)) -> chars)

/*These are the identifiers type which help identify the Object*/
typedef enum {
    OBJ_STRING,
} ObjType;

struct Obj {
    //The first tag is an identifier for storing the size !
    ObjType type;
};

struct ObjString {
    //The obj pointer stores the type !
    Obj obj;
    int length;
    char* chars;
};

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