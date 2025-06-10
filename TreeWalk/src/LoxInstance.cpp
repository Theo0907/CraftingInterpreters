#include "LoxInstance.h"

#include "Token.h"
#include "Interpreter.h"
#include "LoxFunction.h"

Object LoxInstance::get(const Token& name)
{
	{
		auto it = fields.find(name.lexeme);
		if (it != fields.end())
			return it->second;
	}
	{
		std::shared_ptr<LoxFunction> method = klass->findMethod(name.lexeme);
		if (method)
			return { method->bind(selfShared) };
	}
	throw RuntimeError(name, "Undefined property '" + name.lexeme + "'.");
}

void LoxInstance::set(const Token& name, const Object& value)
{
	fields[name.lexeme] = value;
}
