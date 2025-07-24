#include <stdio.h>

#include "memory.h"
#include "value.h"

/*Helps with initialising the struct valueArray to empty values*/
void initValueArray(ValueArray* array) {
    array->values = NULL;
    array->capacity = 0;
    array->count = 0;
}

/*Writing values to our array*/
void writeValueArray(ValueArray* array, Value value) {
    if (array->capacity < array->count + 1) {
        int oldCapacity = array->capacity;
        array->capacity = GROW_CAPACITY(oldCapacity);
        array->values = GROW_ARRAY(Value, array->values,
                                 oldCapacity, array->capacity);
    }

    //Store the value at the end and increment the counter
    array->values[array->count] = value;
    array->count++;
}

/*Freeing the memory used by the array*/
void freeValueArray(ValueArray* array) {
    FREE_ARRAY(Value, array->values, array->capacity);
    initValueArray(array);
}

void printValue(Value value) {
  printf("%g", value);
}