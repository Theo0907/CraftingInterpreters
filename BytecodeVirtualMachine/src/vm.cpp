#include "vm.h"

#include "chunk.h"
#include "value.h"
#include "debug.h"
#include "compiler.h"
#include "obj.h"

#include <format>
#include <iostream>

static Value clockNative(int argCount, Value* args) {
	return { (double)clock() / CLOCKS_PER_SEC };
}

VM::VM() : frames(), stack()
{
	resetStack();
	defineNative("clock", clockNative);
}

VM::~VM()
{
	freeObjects();
}

InterpretResult VM::interpret(const std::string& source)
{
	ObjFunction* func = compile(this, source);
	if (func == nullptr)
		return INTERPRET_COMPILE_ERROR;

	resetStack();

	push(Value(func));
	call(func, 0);

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
	char* chars = (char*)malloc(sizeof(char) * (len + 1));
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
	CallFrame* frame = &frames[frameCount - 1];
#define READ_BYTE() (*frame->ip++)
#define READ_CONSTANT() (frame->function->chunk.constants[READ_BYTE()])
#define READ_SHORT() \
	(frame->ip += 2, (uint16_t)((frame->ip[-2] << 8) | frame->ip[-1]))
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
#define READ_STRING() READ_CONSTANT().getStringObj()

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
		disassembleInstruction(&frame->function->chunk, (int)(frame->ip - frame->function->chunk.code.data()));
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
		case OP_POP: pop(); break;
		case OP_GET_LOCAL:
		{
			uint8_t slot = READ_BYTE();
			push(frame->slots[slot]);
			break;
		}
		case OP_GET_GLOBAL:
		{
			ObjString* name = READ_STRING();
			Value value;
			if (!globals.get(name, &value))
			{
				runtimeError(std::format("Undefined variable '{}'.", name->chars));
				return INTERPRET_RUNTIME_ERROR;
			}
			push(value);
			break;
		}
		case OP_DEFINE_GLOBAL:
		{
			ObjString* name = READ_STRING();
			globals.set(name, peek(0));
			pop();
			break;
		}
		case OP_SET_LOCAL:
		{
			uint8_t slot = READ_BYTE();
			frame->slots[slot] = peek(0);
			break;
		}
		case OP_SET_GLOBAL:
		{
			ObjString* name = READ_STRING();
			if (globals.set(name, peek(0)))
			{
				globals.remove(name);
				runtimeError(std::format("Undefined variable '{}'.", name->chars));
				return INTERPRET_RUNTIME_ERROR;
			}
			break;
		}
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
				runtimeError("Operands must be two numbers or two strings.");
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
		case OP_PRINT:
			printValue(pop());
			std::cout << std::endl;
			break;
		case OP_JUMP:
		{
			uint16_t offset = READ_SHORT();
			frame->ip += offset;
			break;
		}
		case OP_JUMP_IF_FALSE:
		{
			uint16_t offset = READ_SHORT();
			if (isFalsey(peek(0)))
				frame->ip += offset;
			break;
		}
		case OP_LOOP:
		{
			uint16_t offset = READ_SHORT();
			frame->ip -= offset;
			break;
		}
		case OP_CALL:
		{
			int argCount = READ_BYTE();
			if (!callValue(peek(argCount), argCount))
				return INTERPRET_RUNTIME_ERROR;
			// Update cached pointer to new environment
			frame = &frames[frameCount - 1];
			break;
		}
		case OP_RETURN:
		{
			Value result = pop();
			--frameCount;
			if (frameCount == 0)
			{
				pop();
				return INTERPRET_OK;
			}
			stackTop = frame->slots;
			push(result);
			frame = &frames[frameCount - 1];
		}
		}
	}
#undef READ_BYTE
#undef READ_CONSTANT
#undef READ_SHORT
#undef READ_STRING
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
	case OBJ_FUNCTION:
	{
		ObjFunction* func = (ObjFunction*)object;
		delete func;
		// No need to delete name string as it should be garbage collected
		break;
	}
	case OBJ_NATIVE:
	{
		ObjNative* native = (ObjNative*)object;
		delete native;
		break;
	}
	case OBJ_STRING:
	{
		ObjString* string = (ObjString*)object;
		free(string->chars);
		delete string;
		break;
	}
	}
}

// TODO: Add more native functions to the VM.
void VM::defineNative(std::string_view name, NativeFn function)
{
	// Pushing and popping values to stack to keep them from beeing garbage collected
	push({ copyString(static_cast<int>(name.length()), name.data(), this) });
	push({ newNative(function, this) });
	globals.set(stack[0].getStringObj(), stack[1]);
	pop();
	pop();
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

bool VM::callValue(Value callee, int argCount)
{
	if (callee.isObj())
	{
		switch (callee.getObjType())
		{
		case OBJ_FUNCTION:
			return call(callee.getFunctionObj(), argCount);
		case OBJ_NATIVE:
		{
			// TODO: Add arity check to native function calls
			// TODO: Add support for runtime error in a native function
			NativeFn native = callee.getNativeFn();
			Value result = native(argCount, stackTop - argCount);
			stackTop -= argCount + 1;
			push(result);
			return true;
		}
		default:
			break;
		}
	}
	runtimeError("Can only call functions and classes.");
	return false;
}

bool VM::call(ObjFunction* function, int argCount)
{
	if (argCount != function->arity)
	{
		runtimeError(std::format("Expected {} arguments but got {}.", function->arity, argCount));
		return false;
	}

	if (frameCount == FRAMES_MAX)
	{	
		runtimeError("Stack overflow.");
		return false;
	}

	CallFrame* frame = &frames[frameCount++];
	frame->function = function;
	frame->ip = function->chunk.code.data();
	frame->slots = stackTop - argCount - 1;
	return true;
}

// TODO: Maybe re-add variadic param to use std::format dynamically instead of relying on doing it in the message
void VM::runtimeError(const std::string& message)
{
	std::cerr << message << std::endl;

	// Print stack frame
	for (int i = frameCount - 1; i >= 0; --i)
	{
		CallFrame* frame = &frames[i];
		ObjFunction* func = frame->function;
		size_t instruction = frame->ip - func->chunk.code.data() - 1; // Show last executed instruction
		std::cerr << std::format("[line {}] in ", func->chunk.lineInfo[instruction]);
		if (func->name == nullptr)
			std::cerr << "script" << std::endl;
		else
			std::cerr << std::format("{}()", func->name->chars) << std::endl;
	}

	resetStack();
}
