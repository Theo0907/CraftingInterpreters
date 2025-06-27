#pragma once

#include "common.h"
#include "value.h"
#include "chunk.h"
#include "table.h"

#include <string>

enum InterpretResult
{
	INTERPRET_OK,
	INTERPRET_COMPILE_ERROR,
	INTERPRET_RUNTIME_ERROR,
};

// This could be a constexpr instead
#define STACK_MAX 256

struct VM
{
	Chunk chunk ;
	uint8_t* ip = nullptr;
	Value				stack[STACK_MAX];
	Value*				stackTop = nullptr;
	Obj*				objects = nullptr;
	Table				strings;

	VM();
	~VM();

	InterpretResult interpret(const std::string& source);
private:
	InterpretResult	run();
	void			resetStack();
	void			freeVM();
	void			freeObjects();
	void			freeObject(Obj* object);

	void			push(Value value);
	Value			pop();
	Value			peek(int dist);

	void			runtimeError(const std::string& message);

	Value			concatenate(Value a, Value b);
};