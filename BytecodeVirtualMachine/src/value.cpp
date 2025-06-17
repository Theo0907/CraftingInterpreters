#include "value.h"

#include <iostream>
#include <format>

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
	}
}
