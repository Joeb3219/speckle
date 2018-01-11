#include <stdlib.h>
#include <string.h>
#include "hashmap.h"

#define HASHMAP_DEFAULT_SIZE 2048

// We use a FNV-1a hash, as described https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
unsigned long long hash(char* key){
        unsigned long long value = (unsigned long long) 14695981039346656037ULL;
        unsigned long long fnvPrime = 1099511628211L;
        int i = 0, length = strlen(key);
        for(i = 0; i < length; i ++){
                value = value ^ key[i];
                value = value * fnvPrime;
        }
        return value;
}

// Generates a hashmap of HASHMAP_DEFAULT_SIZE.
// Hashmaps can store anything as their data section as a pointer.
Hashmap* createHashmap(){
	int i;
	Hashmap* map = malloc(sizeof(Hashmap));
	map->capacity = HASHMAP_DEFAULT_SIZE;
	map->data = malloc(sizeof(long) * HASHMAP_DEFAULT_SIZE);
	map->filled = 0;
	// By default, we set every pointer to NULL. This helps when we later empty it,
	// as we can simply check if the data is NULL.
	for(i = 0; i < map->capacity; i ++) map->data[i] = HASHMAP_UNKNOWN_VALUE;
	return map;
}

// Destroys the hashmap.
// This is equivalent to freeing all of the data pointers and the hashmap itself.
// WARNING: anything inserted into the hashmap will be FREEd.
void destroyHashmap(Hashmap* map){
	int i = 0;
	free(map->data);
	free(map);
}

// Inserts some value at a key in the hashmap.
void hashmapInsert(Hashmap *map, char* key, long value){
	int i;
	map->filled ++;
	if(map->filled == map->capacity) return;
	map->data[hash(key) % map->capacity] = value;
}

// Reads the data at some key in the hashmap.
long hashmapRead(Hashmap *map, char* key){
	// We find the hash, and use modulo to wrap it such that it is within the capacity.
	// IE: if capacity is 5, and hash is 9, our key will be 4.
	return map->data[hash(key) % map->capacity];
}
