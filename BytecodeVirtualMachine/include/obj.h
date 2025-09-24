#pragma once

#include "common.h"
#include "chunk.h"
#include <functional>

enum ObjType
{
	OBJ_FUNCTION,
	OBJ_NATIVE,
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

struct ObjFunction : public Obj
{
	int arity = 0;
	Chunk chunk = Chunk();
	ObjString* name = nullptr;

	ObjFunction() : Obj(OBJ_FUNCTION)
	{
	}
};

using NativeFn = std::function<struct Value(int argCount, struct Value* args)>;

struct ObjNative : public Obj
{
	NativeFn function;

	ObjNative(NativeFn function) : Obj(OBJ_NATIVE), function{ function }
	{
	}
};

ObjFunction* newFunction(struct VM* vm);
ObjNative* newNative(NativeFn function, struct VM* vm);
ObjString* newString(int len, uint32_t hash, char* chars, struct VM* vm);
ObjString* copyString(int len, const char* chars, struct VM* vm);