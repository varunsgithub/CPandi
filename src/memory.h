#ifndef cpandi_memory_h
#define cpandi_memory_h

#include "common.h"
#include "object.h"

#define ALLOCATE(type, count) \
    (type*)reallocate(NULL, 0, sizeof(type) * (count))

#define FREE(type, pointer) reallocate(pointer, sizeof(type), 0)

// The essence of this preprocessor macro is that it takes in the capacity
// and it checks if it is lower than 8 then it puts the capacity as 8
// if not then it just multiplies it by 2
#define GROW_CAPACITY(capacity) \
    ((capacity) < 8 ? 8 : (capacity) * 2)

#define FREE_ARRAY(type, pointer, oldCount) \
    reallocate(pointer, sizeof(type) * (oldCount), 0)

#define GROW_ARRAY(type, pointer, oldCount, newCount) \
    (type*) reallocate(pointer, sizeof(type) * (oldCount), \
    sizeof(type) * (newCount))

//The line breaks in the macro(#define) are representative of line breaks....

/*The re allocate function helps resize the array*/
void* reallocate(void* pointer, size_t oldSize, size_t newSize);

/*The free objects method helps clear memory on heap allocated for the objects*/
void freeObjects();

#endif