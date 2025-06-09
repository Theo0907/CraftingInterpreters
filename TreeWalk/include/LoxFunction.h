#pragma once

#include "LoxCallable.h"

#include "Stmt.hpp"

class LoxFunction :
    public LoxCallable
{
public:
	Function	declaration;

	LoxFunction(Function declaration);

	virtual ~LoxFunction() override;

	virtual Object call(Interpreter& interpreter, const std::list<Object>& arguments) override;
	virtual int arity() const override;
	virtual std::string ToString() const override;

	operator std::string() const;
};

