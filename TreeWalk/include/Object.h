#pragma once

#include <string>
#include <variant>
#include <iostream>

class Object
{
public:
	std::variant<std::string, double, bool, std::monostate> data;
	std::string	GetString() const 
	{
		if (data.index() == 0)
			return std::get<std::string>(data);
		else if (data.index() == 2)
			return std::to_string(GetBool());
		return std::to_string(GetNumber());
	}
	const double&		GetNumber() const { return std::get<double>(data); }
	bool				GetBool() const { return std::get<bool>(data); }
	bool				IsNull() const { return data.index() == 3; }

	Object(const std::string& s) : data{s} 
	{}
	Object(std::string&& s) : data{ std::move(s) }
	{}
	Object(double d) : data{d}
	{}
	Object(bool b) : data{b}
	{}
	Object() = default;
};
