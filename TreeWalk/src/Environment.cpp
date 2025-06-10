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

void Environment::assignAt(int dist, const Token& name, const Object& value)
{
	ancestor(dist)->values[name.lexeme] = value;
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

const Object& Environment::getAt(int dist, const std::string& name)
{
	return ancestor(dist)->values[name];
}

Environment* Environment::ancestor(int distance)
{
	Environment* env = this;
	for (int i = 0; i < distance && env->enclosing; ++i)
		env = env->enclosing.get();
	return env;
}
