#include "value.h"

#include "vm.h"

#include <iostream>
#include <format>

Value::Value(struct VM* vm, int len, char* chars) : type{ VAL_OBJ }
{
	initStringFromPointer(vm, len, chars);
}
Value::Value(struct VM* vm, int len, const char* inChars) : type{ VAL_OBJ }
{
	initStringFromString(vm, len, inChars);
}

void Value::initStringFromPointer(VM* vm, int len, char* chars)
{
	as.obj = new ObjString(len, chars);
	as.obj->next = vm->objects;
	vm->objects = as.obj;
}

void Value::initStringFromString(VM* vm, int len, const char* inChars)
{
	char* chars = (char*)malloc((len + 1) * sizeof(char));
	if (chars == nullptr)
	{
		std::cerr << "Could not allocate string." << std::endl;
		return;
	}
	memcpy(chars, inChars, len);
	chars[len] = '\0';

	initStringFromPointer(vm, len, chars);
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
