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
}

InterpretResult VM::interpret(const std::string& source)
{
	chunk = Chunk();
	if (!compile(source, &chunk))
		return INTERPRET_COMPILE_ERROR;

	ip = chunk.code.data();

	resetStack();
	return run();
}

static bool isFalsey(Value value)
{
	return value.isNil() || value.isBool() && !(bool)value;
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
		case OP_ADD:      BINARY_OP(+); break;
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
