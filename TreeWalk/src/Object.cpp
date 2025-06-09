#include "Object.h"
#include "LoxFunction.h"

bool operator==(const Object& a, const Object& b)
{
	return a.data == b.data;
}

std::string Object::GetString() const
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
	else if (IsFunction())
		return GetFunction()->ToString();

	// TODO: Add LoxCallable to string method

	return "Data type stringify is not implemented";
}
