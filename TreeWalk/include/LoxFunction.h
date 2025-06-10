#pragma once

#include "LoxCallable.h"

#include "Stmt.hpp"

class LoxFunction :
    public LoxCallable
{
public:
	Function	declaration;
	std::shared_ptr<class Environment>	closure;
	bool		isInitializer;

	LoxFunction(Function declaration, const std::shared_ptr<class Environment>& closure, bool isInitializer);

	virtual ~LoxFunction() override;

	virtual Object call(Interpreter& interpreter, const std::list<Object>& arguments) override;
	virtual int arity() const override;
	virtual std::string ToString() const override;

	std::shared_ptr<LoxFunction>	bind(std::shared_ptr<class LoxInstance> instance);

	operator std::string() const;
};

