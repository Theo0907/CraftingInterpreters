#pragma once

#include "Object.h"

#include <unordered_map>
#include <string>
#include <memory>

class Environment
{
private:
	// TODO: this does not have to be an unordered_map anymore, most access is done via index or could be done via index now
	std::unordered_map<std::string, Object> values;

public:
	std::shared_ptr<class Environment>		enclosing;

	Environment() = default;
	Environment(std::shared_ptr<class Environment> enclosing) : enclosing{ enclosing }
	{}

	void			define(const std::string& name, const Object& value);
	void			assign(const class Token& name, const Object& value);
	void			assignAt(int dist, const Token& name, const Object& value);
	const Object&	get(const class Token& name);
	const Object&	getAt(int dist, const std::string& name);
	Environment*	ancestor(int distance);
};

