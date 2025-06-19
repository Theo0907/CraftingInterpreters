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
	char* chars; // TODO: Maybe make it so that the string itself is allocated with the obj? A bit harder to do, but a single alloc instead of two

	ObjString(int len, char* chars) : Obj(OBJ_STRING), length{len}, chars{chars}
	{
	}
};
