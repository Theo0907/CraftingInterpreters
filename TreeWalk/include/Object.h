#pragma once

#include <string>
#include <variant>

class Object
{
public:
	std::variant<std::string, double, std::monostate> data;
	const std::string& GetString() const { return std::get<std::string>(data); }
	const double& GetNumber() const { return std::get<double>(data); }

	Object(const std::string& s) : data{s} 
	{}
	Object(std::string&& s) : data{ std::move(s) }
	{}
	Object(double d) : data{d}
	{}
	Object() = default;
};
