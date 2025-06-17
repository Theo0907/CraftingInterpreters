#pragma once

#include "common.h"

enum ValueType
{
	VAL_BOOL,
	VAL_NIL,
	VAL_NUMBER,
};

struct Value
{
	ValueType type;
	union
	{
		bool boolean;
		double number;
	} as;

	Value(bool b) : type{ VAL_BOOL }, as{ .boolean = b }
	{ }
	Value(double n) : type{ VAL_NUMBER }, as{ .number = n }
	{ }
	Value() : type{ VAL_NIL }, as{ 0 }
	{ }

	bool	isBool() { return type == VAL_BOOL; }
	bool	isNumber() { return type == VAL_NUMBER; }
	bool	isNil() { return type == VAL_NIL; }

	operator bool()
	{
		return as.boolean;
	}
	operator double()
	{
		return as.number;
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
		default: return false;
		}
	}
};

void printValue(Value value);