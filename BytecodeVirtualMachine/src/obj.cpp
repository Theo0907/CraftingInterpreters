#include "Obj.h"

uint32_t ObjString::hashString(const char* key, int len)
{
	// TODO: Try different hashing functions once better profiling is implemented
	// FNV-1a hashing algorithm
	uint32_t hash = 2166136261u;
	for (int i = 0; i < len; ++i)
	{
		hash ^= (uint8_t)key[i];
		hash *= 16777619;
	}

	return hash;
}
