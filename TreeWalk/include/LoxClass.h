#pragma once

#include "LoxCallable.h"

#include <string>
#include <unordered_map>

class LoxClass : public LoxCallable
{
public:
	std::string name;
	std::weak_ptr<LoxClass> selfShared;
	std::unordered_map<std::string, std::shared_ptr<class LoxFunction>> methods;
	std::shared_ptr<LoxClass>	superclass;

	LoxClass(const std::string& name, const std::shared_ptr<LoxClass>& superclass, const std::unordered_map<std::string, std::shared_ptr<LoxFunction>>& methods)
		: name{ name }, superclass{ superclass }, methods{ methods }
	{ }

	operator std::string() const
	{
		return name;
	}
	std::string ToString() const
	{
		return (std::string)*this;
	}

	Object call(Interpreter& interpreter, const std::list<Object>& arguments) override;
	int arity() const override;

	std::shared_ptr<class LoxFunction>	findMethod(const std::string& methodName) const;
};

