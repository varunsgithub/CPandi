#ifndef cpandi_value_h
#define cpandi_value_h

#include "common.h"

/*Creating a new data type for value*/
typedef double Value;

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