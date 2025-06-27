#pragma once

#include "common.h"
#include "value.h"

struct Entry
{
	ObjString* key = nullptr;
	Value value = {};
};

class Table
{
public:
	int count = 0;
	int capacity = 0;
	Entry* entries = nullptr;

	Table();
	~Table();
	void	reset();

	bool	set(ObjString* key, Value val);
	Entry*	find(ObjString* key);
	bool	get(ObjString* key, Value* val);
	bool	remove(ObjString* key);

	ObjString* findKey(const char* chars, int len, uint32_t hash);

	void	adjustCapacity(int newCapacity);
	void	addAll(const Table& other);
};

