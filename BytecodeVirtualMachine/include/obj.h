#pragma once

#include "common.h"

enum ObjType
{
	OBJ_STRING,
};

struct Obj
{
	ObjType type;
	struct Obj* next = nullptr;
};

struct ObjString : public Obj
{
	int length;
	uint32_t hash;
	char* chars; // TODO: Maybe make it so that the string itself is allocated with the obj? A bit harder to do, but a single alloc instead of two

	ObjString(int len, char* chars) : Obj(OBJ_STRING), length{len}, chars{chars}
	{
		hash = hashString(chars, len);
	}
	ObjString(int len, uint32_t hash, char* chars) : Obj(OBJ_STRING), length{len}, hash{hash}, chars{chars}
	{
	}

	static uint32_t hashString(const char* key, int len);
};
