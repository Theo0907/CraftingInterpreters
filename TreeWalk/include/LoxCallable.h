#pragma once

#include "Object.h"

#include <list>

class LoxCallable
{
public:
	// This is an interface
	virtual ~LoxCallable() = default;

	virtual Object	call(const class Interpreter& interpreter, const std::list<Object>& arguments) = 0;
	virtual int		arity() const = 0;
};

