#ifndef _HASHMAP_H_
#define _HASHMAP_H_

	struct Hashmap{
		int capacity;
		int filled;
		long* data;
	};

	typedef struct Hashmap Hashmap;

	unsigned long long hash(char* key);
	Hashmap* createHashmap();
	void hashmapInsert(Hashmap *map, char* key, long value);
	long hashmapRead(Hashmap* map, char* key);
	void destroyHashmap(Hashmap* map);

#endif
