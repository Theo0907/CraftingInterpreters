#include "Environment.h"

#include "Token.h"
#include "Interpreter.h"

void Environment::define(const std::string& name, const Object& value)
{
	values[name] = value;
}

const Object& Environment::get(const Token& name)
{
	auto it = values.find(name.lexeme);
	if (it != values.end())
		return it->second;

	throw RuntimeError(name, "Undefined variable '" + name.lexeme + "'");
}
