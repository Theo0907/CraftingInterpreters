#include "table.h"

#define TABLE_MAX_LOAD 0.75
// Growth factor is 1.5, init at 8 as initial value
#define GROW_CAPACITY(capacity) ((capacity) < 8 ? 8 : static_cast<int>(std::ceil(capacity) * 1.5))

Table::Table()
{

}

Table::~Table()
{
	if (entries != nullptr)
		free(entries);
}

void Table::reset()
{
	count = 0; 
	for (int i = 0; i < capacity; ++i)
	{
		entries[i].key = nullptr;
		entries[i].value = {};
	}
}

bool Table::set(ObjString* key, Value val)
{
	if (count + 1 > capacity * TABLE_MAX_LOAD)
	{
		int newCapacity = GROW_CAPACITY(capacity);
		adjustCapacity(newCapacity);
	}

	Entry* entry = find(key);
	bool isNewKey = entry->key == nullptr;
	if (isNewKey && entry->value.isNil()) // Checking for new key or tombstone
		++count;

	entry->key = key;
	entry->value = val;

	return isNewKey;
}

Entry* Table::find(ObjString* key)
{
	uint32_t index = key->hash % capacity;

	Entry* tombstone = nullptr;

	// This should not need to ever break out since there should always be capacity remaining
	while (true)
	{
		Entry* entry = &entries[index];
		// This will always be true at some point as there is always remaining capacity
		// This only matches for same key pointer, maybe need to match for its value too?
		if (entry->key == key)
			return entry;
		else if (entry->key == nullptr)
		{
			if (entry->value.isNil()) // Empty entry, return a tombstone if one was found as they are closer to desired value.
				return tombstone != nullptr ? tombstone : entry;
			else // Found a tombstone 
				if (tombstone == nullptr)
					tombstone = entry;
		}

		index = (index + 1) % capacity;
	}
}

bool Table::get(ObjString* key, Value* val)
{
	if (count == 0)
		return false;

	Entry* entry = find(key);
	if (entry->key == nullptr)
		return false;

	*val = entry->value;
	return true;
}

bool Table::remove(ObjString* key)
{
	if (count == 0)
		return false;

	Entry* entry = find(key);
	if (entry->key == nullptr)
		return false;

	entry->key = nullptr;
	entry->value = { true };
	return true;
}

ObjString* Table::findKey(const char* chars, int len, uint32_t hash)
{
	if (count == 0)
		return nullptr;

	uint32_t index = hash % capacity;
	while (true) // 
	{
		Entry* entry = &entries[index];
		if (entry->key == nullptr)
		{
			if (entry->value.isNil())
				return nullptr;
		}
		else if (entry->key->length == len && entry->key->hash == hash && memcmp(entry->key->chars, chars, len) == 0)
			return entry->key;

		index = (index + 1) % capacity;
	}
}

void Table::adjustCapacity(int newCapacity)
{
	Entry* oldEntries = entries;
	entries = (Entry*)malloc(sizeof(Entry) * newCapacity);
	int oldCapacity = capacity;
	capacity = newCapacity;
	count = 0;

	for (int i = 0; i < capacity; ++i)
	{
		entries[i].key = nullptr;
		entries[i].value = {};
	}

	for (int i = 0; i < oldCapacity; ++i)
	{
		Entry* entry = &oldEntries[i];
		if (entry->key == nullptr)
			continue;
		Entry* dest = find(entry->key);
		dest->key = entry->key;
		dest->value = entry->value;
		++count;
	}

	if (oldEntries != nullptr)
		free(oldEntries);
}

void Table::addAll(const Table& other)
{
	for (int i = 0; i < other.capacity; ++i)
	{
		Entry* entry = &other.entries[i];
		if (entry->key != nullptr)
			set(entry->key, entry->value);
	}
}
