#include "LoxClass.h"

#include "LoxInstance.h"
#include "LoxFunction.h"

Object LoxClass::call(Interpreter& interpreter, const std::list<Object>& arguments)
{
	std::shared_ptr<LoxInstance> instance = std::make_shared<LoxInstance>(selfShared.lock());
	instance->selfShared = instance;
	std::shared_ptr<LoxFunction> initializer = findMethod("init");
	if (initializer)
		initializer->bind(instance)->call(interpreter, arguments);
	return { instance };
}

int LoxClass::arity() const
{
	std::shared_ptr<LoxFunction> initializer = findMethod("init");
	if (initializer)
		return initializer->arity();
    return 0;
}

std::shared_ptr<class LoxFunction> LoxClass::findMethod(const std::string& methodName) const
{
	auto it = methods.find(methodName);
	if (it != methods.end())
		return it->second;

	return nullptr;
}
