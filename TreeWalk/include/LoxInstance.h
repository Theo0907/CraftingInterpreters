#pragma once

#include "LoxClass.h"

#include <unordered_map>

class LoxInstance
{
public:
	std::shared_ptr<LoxInstance> selfShared;
	std::shared_ptr<LoxClass> klass;
	std::unordered_map<std::string, Object> fields;

	LoxInstance(const std::shared_ptr<LoxClass>& klass) : klass(klass)
	{
	}

	operator std::string() const
	{
		return klass->name + " instance";
	}

	std::string ToString() const
	{
		return (std::string)*this;
	}

	Object	get(const class Token& name);
	void	set(const class Token& name, const Object& value);
};

