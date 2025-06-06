#include "Environment.h"

#include "Token.h"
#include "Interpreter.h"

void Environment::define(const std::string& name, const Object& value)
{
	values[name] = value;
}

void Environment::assign(const Token& name, const Object& value)
{
	auto it = values.find(name.lexeme);
	if (it != values.end())
	{
		it->second = value;
		return;
	}
	
	if (enclosing)
	{
		enclosing->assign(name, value);
		return;
	}

	throw RuntimeError(name, "Undefined variable '" + name.lexeme + "'.");
}

const Object& Environment::get(const Token& name)
{
	auto it = values.find(name.lexeme);
	if (it != values.end())
		return it->second;

	if (enclosing)
		return enclosing->get(name);

	throw RuntimeError(name, "Undefined variable '" + name.lexeme + "'");
}
