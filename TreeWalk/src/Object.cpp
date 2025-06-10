#include "Object.h"
#include "LoxFunction.h"
#include "LoxClass.h"
#include "LoxInstance.h"

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
	else if (IsClass())
		return GetClass()->ToString();
	else if (IsInstance())
		return GetInstance()->ToString();

	// TODO: Add LoxCallable to string method

	return "Data type stringify is not implemented";
}

std::shared_ptr<LoxCallable> Object::GetFunction() const
{
	if (IsFunction())
		return std::get<std::shared_ptr<LoxCallable>>(data);
	else if (IsClass())
		return std::dynamic_pointer_cast<LoxCallable>(GetClass());

	return {};
}
