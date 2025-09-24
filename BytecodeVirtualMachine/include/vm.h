#pragma once

#include "common.h"
#include "value.h"
#include "chunk.h"
#include "table.h"
#include "obj.h"

#include <string>
#include <array>

enum InterpretResult
{
	INTERPRET_OK,
	INTERPRET_COMPILE_ERROR,
	INTERPRET_RUNTIME_ERROR,
};

// This could be a constexpr instead
#define FRAMES_MAX 64
#define STACK_MAX (FRAMES_MAX * UINT8_COUNT)

struct CallFrame
{
	ObjFunction* function;
	uint8_t* ip;
	Value* slots;
};

struct VM
{
	std::array<CallFrame, FRAMES_MAX>	frames;
	int									frameCount = 0;

	Value				stack[STACK_MAX];
	Value*				stackTop = nullptr;
	Obj*				objects = nullptr;
	Table				strings;
	// TODO: Put globals in an array to be able to access them faster (not needing a search every time a value is needed)
	Table				globals;

	VM();
	~VM();

	InterpretResult interpret(const std::string& source);
private:
	InterpretResult	run();
	void			resetStack();
	void			freeVM();
	void			freeObjects();
	void			freeObject(Obj* object);
	void			defineNative(std::string_view name, NativeFn function);

	void			push(Value value);
	Value			pop();
	Value			peek(int dist);
	bool			callValue(Value callee, int argCount);
	bool			call(ObjFunction* function, int argCount);

	void			runtimeError(const std::string& message);

	Value			concatenate(Value a, Value b);
};