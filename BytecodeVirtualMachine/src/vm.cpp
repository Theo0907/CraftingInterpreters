#include "vm.h"

#include "chunk.h"
#include "value.h"
#include "debug.h"
#include "compiler.h"

#include <format>
#include <iostream>

VM::VM() : stack()
{
}

VM::~VM()
{
	freeObjects();
}

InterpretResult VM::interpret(const std::string& source)
{
	chunk = Chunk();
	if (!compile(this, source, &chunk))
		return INTERPRET_COMPILE_ERROR;

	ip = chunk.code.data();

	resetStack();
	return run();
}

static bool isFalsey(Value value)
{
	return value.isNil() || value.isBool() && !(bool)value;
}

Value VM::concatenate(Value a, Value b)
{
	ObjString* strA = static_cast<ObjString*>(a.as.obj);
	ObjString* strB = static_cast<ObjString*>(b.as.obj);

	int len = strA->length + strB->length;
	char* chars = (char*)malloc(sizeof(char) * len);
	if (chars == nullptr)
	{
		std::cerr << "Could not allocate string" << std::endl;
		return {};
	}
	memcpy(chars, strA->chars, strA->length);
	memcpy(chars + strA->length, strB->chars, strB->length);
	chars[len] = '\0';

	return { this, len, chars };
}

InterpretResult VM::run()
{
#define READ_BYTE() (*ip++)
#define READ_CONSTANT() (chunk.constants[READ_BYTE()])
#define BINARY_OP(op)\
	do {\
		if (!peek(0).isNumber() || !peek(1).isNumber())\
		{\
			runtimeError("Operands must be numbers.");\
			return INTERPRET_RUNTIME_ERROR;\
		}\
		double b = pop();\
		double a = pop();\
		push(a op b);\
	} while (false)

	while (true)
	{
#ifdef DEBUG_TRACE_EXECUTION
		std::cout << "          " << std::endl;
		for (Value* slot = stack; slot < stackTop; ++slot)
		{
			std::cout << "[ ";
			printValue(*slot);
			std::cout << " ]";
		}
		std::cout << std::endl;
		disassembleInstruction(&chunk, (int)(ip - chunk.code.data()));
#endif
		uint8_t instruction;
		switch (instruction = READ_BYTE())
		{
		case OP_CONSTANT:
			push(READ_CONSTANT());
			break;
		case OP_NIL: push({}); break;
		case OP_TRUE: push(true); break;
		case OP_FALSE: push(false); break;
		case OP_EQUAL: 
		{
			Value b = pop();
			Value a = pop();
			push(a == b);
			break;
		}
		case OP_GREATER:  BINARY_OP(>); break;
		case OP_LESS:     BINARY_OP(<); break;
		case OP_ADD:
		{
			if (peek(0).isString() && peek(1).isString())
			{
				Value b = pop();
				push(concatenate(pop(), b));
				break;
			}
			else if (peek(0).isNumber() && peek(1).isNumber())
			{
				BINARY_OP(+);
				break;
			}
			else
			{
				runtimeError("Operands must be two numbers or two string.");
				return INTERPRET_RUNTIME_ERROR;
			}
		}
		case OP_SUBTRACT: BINARY_OP(-); break;
		case OP_MULTIPLY: BINARY_OP(*); break;
		case OP_DIVIDE:   BINARY_OP(/); break;
		case OP_NOT:
			push(isFalsey(pop()));
			break;
		case OP_NEGATE:
			if (!peek(0).isNumber())
			{
				runtimeError("Operand must be a number.");
				return INTERPRET_RUNTIME_ERROR;
			}
			push(-(double)pop());break;
		case OP_RETURN:
			printValue(pop());
			std::cout << std::endl;
			return INTERPRET_OK;
		}
	}
#undef READ_BYTE
#undef READ_CONSTANT
#undef BINARY_OP
}

void VM::resetStack()
{
	stackTop = stack;
}

void VM::freeVM()
{
	freeObjects();
}

void VM::freeObjects()
{
	Obj* object = objects;
	while (object != nullptr)
	{
		Obj* next = object->next;
		freeObject(object);
		object = next;
	}
	objects = nullptr;
}

void VM::freeObject(Obj* object)
{
	switch (object->type)
	{
	case OBJ_STRING:
	{
		ObjString* string = (ObjString*)object;
		free(string->chars);
		delete string;
	}
	}
}

void VM::push(Value value)
{
	(*stackTop++) = value;
}

Value VM::pop()
{
	return (*--stackTop);
}

Value VM::peek(int dist)
{
	// Stack top is not the first element but the next element to be executed, it will always be at least index 1 as long as there is something in the stack
	return stackTop[-1-dist];
}

// TODO: Maybe re-add variadic param to use std::format dynamically instead of relying on doing it in the message
void VM::runtimeError(const std::string& message)
{
	std::cerr << message << std::endl;

	size_t instruction = ip - chunk.code.data() - 1;
	int line = chunk.lineInfo[instruction];
	std::cerr << std::format("[line {}] in script\n", line);
	resetStack();
}
