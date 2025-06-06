#pragma once

#include "Object.h"

#include <unordered_map>
#include <string>
#include <memory>

class Environment
{
private:
	std::unordered_map<std::string, Object> values;

public:
	std::shared_ptr<class Environment>		enclosing;

	Environment() = default;
	Environment(std::shared_ptr<class Environment> enclosing) : enclosing{ enclosing }
	{}

	void			define(const std::string& name, const Object& value);
	void			assign(const class Token& name, const Object& value);
	const Object&	get(const class Token& name);
};

