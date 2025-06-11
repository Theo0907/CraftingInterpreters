#include "value.h"

#include <iostream>
#include <format>

void printValue(Value value)
{
	std::cout << std::format("'{:g}'", value);
}
