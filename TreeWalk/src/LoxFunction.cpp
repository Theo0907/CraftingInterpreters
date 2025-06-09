#include "LoxFunction.h"

#include "Interpreter.h"

LoxFunction::LoxFunction(Function declaration) : declaration{ declaration }
{
}

LoxFunction::~LoxFunction()
{
}

Object LoxFunction::call(Interpreter& interpreter, const std::list<Object>& arguments)
{
	std::shared_ptr<Environment> env = std::make_shared<Environment>(interpreter.globals);

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
		return returnValue.value;
	}

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

LoxFunction::operator std::string() const
{
	return "<fn " + declaration.name->lexeme + ">";
}
