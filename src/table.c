#include <stdlib.h>
#include <string.h>

#include "memory.h"
#include "object.h"
#include "table.h"
#include "value.h"

#define TABLE_MAX_LOAD 0.75

void initTable(Table* table) {
    table->count = 0;
    table->capacity = 0;
    table->entries = NULL;
}

void freeTable(Table* table) {
    FREE_ARRAY(Entry, table->entries, table->capacity);
    initTable(table);
}

/*The find entry takes an array of entries, capacity and the key to find the hashing position for a key*/
static Entry* findEntry(Entry* entries, int capacity, ObjString* key) {
    //First get the hash code from the key
    uint32_t index = key->hash % capacity;
    for (;;) {
        //store a reference to that particular index point
        Entry* entry = &entries[index];
        //if the entry matches the key/ Is null then return the entry else
        if (entry->key == key || entry->key == NULL) {
            return entry;
        }
        // increment the index by 1 and find the new index !
        index = (index + 1) % capacity;
    }
}

/*This method helps allocate the entries and fits that into the table*/
static void adjustCapacity(Table* table, int capacity) {
    /*Entries are first allocated with the entry and capacity*/
    Entry* entries = ALLOCATE(Entry, capacity);

    for (int i = 0; i < capacity; i++) {
        //The key is set to NULL
        entries[i].key = NULL;
        //The values are nil
        entries[i].value = NIL_VAL;
    }

    //fitiing this empty entries and values in the hash table
    table->entries = entries;
    table->capacity = capacity;
}




bool tableSet(Table* table, ObjString* key, Value value) {
    //If the table's elements are > 75% of the capacity
    //then grow the capacity.
    if (table->count + 1 > table->capacity * TABLE_MAX_LOAD) {
        int capacity = GROW_CAPACITY(table->capacity);
        adjustCapacity(table, capacity);
    }


    Entry* entry = findEntry(table->entries, table->capacity, key);
    bool isNewKey = entry->key == NULL;
    
    if (isNewKey) table->count++;

    
    entry->key = key;
    
    entry->value = value;
    
    return isNewKey;
}