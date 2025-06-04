#pragma once

#include <string>
#include <variant>
#include <iostream>

class Object
{
public:
	std::variant<std::string, double, std::monostate> data;
	std::string	GetString() const 
	{
		if (data.index() == 0)
			return std::get<std::string>(data);
		std::cout << "Number: " << GetNumber() << " String: " << std::to_string(GetNumber()) << std::endl;
		return std::to_string(GetNumber());
	}
	const double&		GetNumber() const { return std::get<double>(data); }
	bool				IsNull() const { return data.index() == 2; }

	Object(const std::string& s) : data{s} 
	{}
	Object(std::string&& s) : data{ std::move(s) }
	{}
	Object(double d) : data{d}
	{}
	Object() = default;
};
