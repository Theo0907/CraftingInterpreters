#pragma once

#include <string>
#include <variant>
#include <iostream>
#include <sstream>
#include <iomanip>

class Object
{
public:
	std::variant<std::string, double, bool, std::shared_ptr<class LoxCallable>, std::shared_ptr<class LoxClass>, std::shared_ptr<class LoxInstance>, std::monostate> data;

	std::string	GetString() const;
	const double&		GetNumber() const { return std::get<double>(data); }
	bool				GetBool() const { return std::get<bool>(data); }
	std::shared_ptr<class LoxCallable>	GetFunction() const; // Need to return a function interface, can be other objects too
	std::shared_ptr<class LoxClass>	GetClass() const { return std::get<std::shared_ptr<class LoxClass>>(data); }
	std::shared_ptr<class LoxInstance>	GetInstance() const { return std::get<std::shared_ptr<class LoxInstance>>(data); }

	size_t				GetObjectTypeIndex() { return data.index(); }

	bool				IsString() const { return data.index() == 0; }
	bool				IsNumber() const { return data.index() == 1; }
	bool				IsBool() const { return data.index() == 2; }
	bool				IsFunction() const { return data.index() == 3; }
	bool				IsClass() const { return data.index() == 4; }
	bool				IsInstance() const { return data.index() == 5; }
	bool				IsNull() const { return data.index() == 6; }

	Object(const std::string& s) : data{s} 
	{}
	Object(std::string&& s) : data{ std::move(s) }
	{}
	Object(double d) : data{d}
	{}
	Object(bool b) : data{b}
	{}
	Object(std::shared_ptr<class LoxCallable> lc) : data{lc}
	{ }
	Object(std::shared_ptr<class LoxClass> lc) : data{lc}
	{ }
	Object(std::shared_ptr<class LoxInstance> li) : data{li}
	{ }
	Object() : data{ std::monostate() }
	{}
};

bool	operator==(const Object& a, const Object& b);
