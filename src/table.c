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

    Entry* tombstone = NULL;

    for (;;) {
        //store a reference to that particular index point
        Entry* entry = &entries[index];
        
        if (entry->key == NULL) {
            if (IS_NIL(entry->value)) {
                //entry (empty)
                return tombstone != NULL ? tombstone : entry;
            } else {
                //we got a tombstone
                if (tombstone == NULL) tombstone = entry;
            }
        }   else if (entry->key == key) {
            // found a key
            return entry;
        }

        // increment the index by 1 and find the new index !
        index = (index + 1) % capacity;
    }
}

bool tableGet(Table* table, ObjString* key, Value* value) {
    //if the table count is 0, then return false
    if (table->count == 0) return false;
    
    //if you find an entry that is not null then skip to line 46 and return true
    Entry* entry = findEntry(table->entries, table->capacity, key);
    if (entry->key == NULL) return false;

    *value = entry->value;
    return true;
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

    //invalidate the size
    table->count = 0;

    for (int i = 0; i < table->capacity; i++) {
        //First get the entry 
        Entry* entry = &table->entries[i];
        //this is a tombstone marker
        if (entry->key == NULL) continue;

        //What the entry pointer does is it finds the entries based on the 
        //new entries pointer and stores the old keys and values in it
        Entry* dest = findEntry(entries, capacity, entry->key);
        dest->key = entry->key;
        dest->value = entry->value;
        table->count++;
    }

    FREE_ARRAY(Entry, table->entries, table->capacity);

    //fitting this empty entries and values in the hash table
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
    //this helps increment the count only when a fresh bucket is overwritten 
    //and not tombstones
    if (isNewKey && IS_NIL(entry->value)) table->count++;

    entry->key = key;
    entry->value = value;
    return isNewKey;
}

bool tableDelete(Table* table, ObjString* key) {
    if (table->count == 0) return false;

    //find the entry
    Entry* entry = findEntry(table->entries, table->capacity, key);
    if (entry->key == NULL) return false;

    //place a tombstone in the entry
    entry->key == NULL;
    entry->value = BOOL_VAL(true);
    return true;
}

void tableAddAll(Table* from, Table* to) {
    for (int i = 0; i < from->capacity; i++) {
        Entry* entry = &from->entries[i];
        if (entry->key != NULL) {
            tableSet(to, entry->key, entry->value);
        }
    }
}


ObjString* tableFindString(Table* table, const char* chars,
                           int length, uint32_t hash) {
  if (table->count == 0) return NULL;

  uint32_t index = hash % table->capacity;
  for (;;) {
    Entry* entry = &table->entries[index];
    if (entry->key == NULL) {
      // Stop if we find an empty non-tombstone entry.
      if (IS_NIL(entry->value)) return NULL;
    } else if (entry->key->length == length &&
        entry->key->hash == hash &&
        memcmp(entry->key->chars, chars, length) == 0) {
      // We found it.
      return entry->key;
    }

    index = (index + 1) % table->capacity;
  }
}