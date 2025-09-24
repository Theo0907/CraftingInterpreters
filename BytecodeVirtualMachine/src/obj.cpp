#include "obj.h"

#include "vm.h"

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

ObjFunction* newFunction(VM* vm)
{
	ObjFunction* function = new ObjFunction();
	function->arity = 0;
	function->name = NULL;

	function->next = vm->objects;
	vm->objects = function;
	
	return function;
}

ObjNative* newNative(NativeFn function, VM* vm)
{
	ObjNative* native = new ObjNative(function);
	native->next = vm->objects;
	vm->objects = native;

	return native;
}

ObjString* copyString(int len, const char* inChars, VM* vm)
{
	// Copy string from input
	char* chars = (char*)malloc((len + 1) * sizeof(char));
	if (chars == nullptr)
	{
		std::cerr << "Could not allocate string." << std::endl;
		return nullptr;
	}
	memcpy(chars, inChars, len);
	chars[len] = '\0';

	return newString(len, ObjString::hashString(chars, len), chars, vm);
}

ObjString* newString(int len, uint32_t hash, char* chars, VM* vm)
{
	ObjString* string = new ObjString(len, hash, chars);
	string->next = vm->objects;
	vm->objects = string;
	vm->strings.set(string, {});

	return string;
}
