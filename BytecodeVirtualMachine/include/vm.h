#pragma once

#include "common.h"
#include "value.h"

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
	struct Chunk* chunk = nullptr;
	uint8_t* ip = nullptr;
	Value				stack[STACK_MAX];
	Value*				stackTop = nullptr;

	VM();
	~VM();

	InterpretResult interpret(struct Chunk* chunk);
private:
	InterpretResult	run();
	void			resetStack();

	void			push(Value value);
	Value			pop();
};