#ifndef cpandi_table_h
#define cpandi_table_h

#include "common.h"
#include "value.h"

/*Each entry has a string key and a Value associated with it*/
typedef struct {
    ObjString* key;
    Value value;
} Entry;

/*The table is a hash table with a count, capacity and the entries*/
typedef struct {
    // The count of the number of items in the table
    int count;
    // The capacity of the total hash table (count/ capacity = load factor)
    int capacity;
    // The values inside a hash table is an array of entries
    Entry* entries;
} Table;

/*This method is used to initialize the hashtable*/
void initTable(Table* table);

/*This method is used to free the hashtable*/
void freeTable(Table* table);

/*This method helps fill the hash table with entries*/
bool tableSet(Table* table, ObjString* key, Value value);


#endif