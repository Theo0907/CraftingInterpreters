#include "value.h"

#include "vm.h"

#include <iostream>
#include <format>

Value::Value(struct VM* vm, int len, char* chars) : type{ VAL_OBJ }
{
	uint32_t hash = ObjString::hashString(chars, len);
	ObjString* interned = vm->strings.findKey(chars, len, hash);
	if (interned != nullptr) // If interned string already exists, use it and free input string
	{
		as.obj = interned;
		free(chars);
	}
	else
		initStringFromPointer(vm, len, chars);
}
Value::Value(struct VM* vm, int len, const char* inChars) : type{ VAL_OBJ }
{
	uint32_t hash = ObjString::hashString(inChars, len);
	ObjString* interned = vm->strings.findKey(inChars, len, hash);
	if (interned != nullptr) // If interned string already exists, use it.,
		as.obj = interned;
	else
		initStringFromString(vm, len, inChars);
}

void Value::initStringFromPointer(VM* vm, int len, char* chars, uint32_t* optionalHash)
{
	// Use input string directly
	uint32_t hash;
	if (optionalHash == nullptr)
		hash = ObjString::hashString(chars, len);
	else
		hash = *optionalHash;
	ObjString* string = new ObjString(len, hash, chars);
	as.obj = string;
	as.obj->next = vm->objects;
	vm->objects = as.obj;
	vm->strings.set(string, {});
}

void Value::initStringFromString(VM* vm, int len, const char* inChars, uint32_t* optionalHash)
{
	// Copy string from input
	char* chars = (char*)malloc((len + 1) * sizeof(char));
	if (chars == nullptr)
	{
		std::cerr << "Could not allocate string." << std::endl;
		return;
	}
	memcpy(chars, inChars, len);
	chars[len] = '\0';

	initStringFromPointer(vm, len, chars, optionalHash);
}

void printValue(Value value)
{
	switch (value.type)
	{
	case VAL_BOOL:
		std::cout << ((bool)value ? "true" : "false"); break;
	case VAL_NIL:
		std::cout << "nil"; break;
	case VAL_NUMBER:
		std::cout << std::format("'{:g}'", (double)value); break;
	case VAL_OBJ:
		printObject(value); break;
	}
}

void printObject(Obj* object)
{
	switch (object->type)
	{
	case OBJ_STRING:
		ObjString* str = static_cast<ObjString*>(object);
		std::cout << str->chars;
		break;
	}
}
