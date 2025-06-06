#pragma once

#include "Object.h"

#include <unordered_map>
#include <string>

class Environment
{
private:
	std::unordered_map<std::string, Object> values;

public:
	void			define(const std::string& name, const Object& value);
	void			assign(const class Token& name, const Object& value);
	const Object&	get(const class Token& name);
};

