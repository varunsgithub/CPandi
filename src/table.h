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

/*This is used to retrieve stuff from the hash table*/
bool tableGet(Table* table, ObjString* key, Value* value);

/*This method helps fill the hash table with entries*/
bool tableSet(Table* table, ObjString* key, Value value);

/*This method helps delete the entries from a table, places tombstones on them*/
bool tableDelete(Table* table, ObjString* key);

/*This method helps copy all the entries of the hashtable into a new table*/
void tableAddAll(Table* from, Table* to);

ObjString* tableFindString(Table* table, const char* chars, int length, uint32_t hash);

#endif