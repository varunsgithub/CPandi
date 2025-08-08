#ifndef cpandi_value_h
#define cpandi_value_h

#include "common.h"

/*Enum for defininig the Value types for the virtual machine*/
typedef enum {
  VAL_BOOL,
  VAL_NIL,
  VAL_NUMBER,
} ValueType;

/*The value struct stores the union structure for boolean and number !*/
typedef struct {
  ValueType type;
  union {
    bool boolean;
    double number;
  } as;
} Value;

//Checks for the values, whether a value type matches the given type !
#define IS_BOOL(value)    ((value).type == VAL_BOOL)
#define IS_NIL(value)     ((value).type == VAL_NIL)
#define IS_NUMBER(value)  ((value).type == VAL_NUMBER)

//Used to fetch the respective values
#define AS_BOOL(value)    ((value).as.boolean)
#define AS_NUMBER(value)  ((value).as.number)

//These macros are used to cast the values to their respective value type :)
#define BOOL_VAL(value)   ((Value){VAL_BOOL, {.boolean = value}})
#define NIL_VAL           ((Value){VAL_NIL, {.number = 0}})
#define NUMBER_VAL(value) ((Value){VAL_NUMBER, {.number = value}})


// /*Creating a new data type for value*/
// typedef double Value;

/*Creating a new Struct to hold the constant pool
  which has the capacity, count and the value array*/
typedef struct {
    int capacity;
    int count;
    Value* values;
} ValueArray;

/*Since we have a pointer array, we need a way to initialize, write and free it*/

void initValueArray(ValueArray* array);
void writeValueArray(ValueArray* array, Value value);
void freeValueArray(ValueArray* array);
void printValue(Value value);

#endif