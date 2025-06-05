#pragma once

#include <string>
#include <variant>
#include <iostream>
#include <sstream>
#include <iomanip>

class Object
{
public:
	std::variant<std::string, double, bool, std::monostate> data;
	std::string	GetString() const 
	{
		if (IsString())
			return std::get<std::string>(data);
		else if (IsBool())
			return std::to_string(GetBool());
		else if (IsNumber())
		{
			std::ostringstream oss;
			oss << std::setprecision(8) << std::noshowpoint << GetNumber();
			return oss.str();
		}
		else if (IsNull())
			return "nil";

		return "Data type stringify is not implemented";
	}
	const double&		GetNumber() const { return std::get<double>(data); }
	bool				GetBool() const { return std::get<bool>(data); }

	size_t				GetObjectTypeIndex() { return data.index(); }

	bool				IsString() const { return data.index() == 0; }
	bool				IsNumber() const { return data.index() == 1; }
	bool				IsBool() const { return data.index() == 2; }
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

bool	operator==(const Object& a, const Object& b);
