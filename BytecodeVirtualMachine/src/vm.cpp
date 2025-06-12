#include "vm.h"

#include "chunk.h"
#include "value.h"
#include "debug.h"
#include "compiler.h"

#include <iostream>

VM::VM() : stack()
{
}

VM::~VM()
{
}

InterpretResult VM::interpret(const std::string& source)
{
	compile(source);
	return INTERPRET_OK;
	/*chunk = inChunk;
	ip = chunk->code.data();

	resetStack();
	return run();*/
}

InterpretResult VM::run()
{
#define READ_BYTE() (*ip++)
#define READ_CONSTANT() (chunk->constants[READ_BYTE()])
#define BINARY_OP(op)\
	do {\
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
		disassembleInstruction(chunk, (int)(ip - chunk->code.data()));
#endif
		uint8_t instruction;
		switch (instruction = READ_BYTE())
		{
		case OP_CONSTANT:
			push(READ_CONSTANT());
			break;
		case OP_ADD:      BINARY_OP(+); break;
		case OP_SUBTRACT: BINARY_OP(-); break;
		case OP_MULTIPLY: BINARY_OP(*); break;
		case OP_DIVIDE:   BINARY_OP(/); break;
		case OP_NEGATE:push(-pop());break;
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
