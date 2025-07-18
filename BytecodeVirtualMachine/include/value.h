#pragma once

#include "common.h"
#include "Obj.h"

#include <string>
#include <iostream>

enum ValueType
{
	VAL_BOOL,
	VAL_NIL,
	VAL_NUMBER,
	VAL_OBJ,
};

struct Obj;

struct Value
{
	ValueType type;
	union
	{
		bool	boolean;
		double	number;
		Obj* obj;
	} as;

	Value(bool b) : type{ VAL_BOOL }, as{ .boolean = b }
	{
	}
	Value(double n) : type{ VAL_NUMBER }, as{ .number = n }
	{
	}
	Value(Obj* o) : type{ VAL_OBJ }, as{ .obj = o }
	{
	}
	Value() : type{ VAL_NIL }, as{ 0 }
	{
	}
	Value(struct VM* vm, int len, char* chars);
	Value(struct VM* vm, int len, const char* inChars);
	Value(struct VM* vm, const std::string_view& stringview) : Value(vm, (int)stringview.length(), stringview.data())
	{

	}
	Value(struct VM* vm, const std::string& string) : Value(vm, (int)string.length(), string.c_str())
	{
		
	}
	bool	isBool() { return type == VAL_BOOL; }
	bool	isNumber() { return type == VAL_NUMBER; }
	bool	isNil() { return type == VAL_NIL; }
	bool	isObj() { return type == VAL_OBJ; }

	// Object functions
	ObjType	getObjType() { return as.obj->type; }
	bool	isString() { return isObj() && getObjType() == OBJ_STRING; }
	ObjString* getStringObj() { return static_cast<ObjString*>(as.obj); }
	int		getStringLen() { return static_cast<ObjString*>(as.obj)->length; }
	char*	getStringChars() { return static_cast<ObjString*>(as.obj)->chars; }
	void	initStringFromPointer(struct VM* vm, int len, char* chars, uint32_t* optionalHash = nullptr);
	void	initStringFromString(struct VM* vm, int len, const char* inChars, uint32_t* optionalHash = nullptr);

	operator bool()
	{
		return as.boolean;
	}
	operator double()
	{
		return as.number;
	}
	operator Obj*()
	{
		return as.obj;
	}

	bool operator==(Value other)
	{
		if (type != other.type)
			return false;

		switch (type)
		{
		case VAL_BOOL: return as.boolean == (bool)other;
		case VAL_NIL: return true;
		case VAL_NUMBER: return as.number == (double)other;
		case VAL_OBJ: return as.obj == other.as.obj;
		default: return false;
		}
	}
};

void printValue(Value value);
void printObject(Obj* object);