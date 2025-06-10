#include "LoxFunction.h"

#include "Interpreter.h"

LoxFunction::LoxFunction(Function declaration, const std::shared_ptr<class Environment>& closure, bool isInitializer) 
	: declaration{ declaration }, closure{ closure }, isInitializer{ isInitializer }
{
}

LoxFunction::~LoxFunction()
{
}

Object LoxFunction::call(Interpreter& interpreter, const std::list<Object>& arguments)
{
	std::shared_ptr<Environment> env = std::make_shared<Environment>(closure);

	{
		auto param = declaration.params.begin();
		auto arg = arguments.begin();

		while (param != declaration.params.end() && arg != arguments.end())
		{
			env->define(param->lexeme, *arg);
			++param;
			++arg;
		}
	}

	try
	{
		interpreter.executeBlock(declaration.body, env);
	}
	catch (Interpreter::ReturnException returnValue)
	{
		if (isInitializer)
			return closure->getAt(0, "this");
		return returnValue.value;
	}

	if (isInitializer)
		return closure->getAt(0, "this");
	return {};
}

int LoxFunction::arity() const
{
	return (int)declaration.params.size();
}

std::string LoxFunction::ToString() const
{
	return (std::string)*this;
}

std::shared_ptr<LoxFunction> LoxFunction::bind(std::shared_ptr<class LoxInstance> instance)
{
	std::shared_ptr<Environment> env = std::make_shared<Environment>(closure);
	env->define("this", instance);
	return std::make_shared<LoxFunction>(declaration, env, isInitializer);
}

LoxFunction::operator std::string() const
{
	return "<fn " + declaration.name->lexeme + ">";
}
