#pragma once

#include "common.h"
#include "value.h"

#include <vector>

enum OpCode : uint8_t
{
	OP_CONSTANT,
	OP_NIL,
	OP_TRUE,
	OP_FALSE,
	OP_POP,
	OP_GET_GLOBAL,
	OP_DEFINE_GLOBAL,
	OP_SET_GLOBAL,
	OP_EQUAL,
	OP_GREATER,
	OP_LESS, // TODO: Create instructions for != <= >= for speed of execution
	OP_ADD,
	OP_SUBTRACT,
	OP_MULTIPLY,
	OP_DIVIDE,
	OP_NOT,
	OP_NEGATE,
	OP_PRINT,
	OP_RETURN,
};

struct Chunk
{
	std::vector<uint8_t>	code;
	std::vector<Value>		constants;
	std::vector<int>		lineInfo;

	Chunk();
	void	write(uint8_t byte, int line);
	int		addConstant(Value v);
};