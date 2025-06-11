#pragma once

#include "common.h"
#include "value.h"

#include <vector>

enum OpCode : uint8_t
{
	OP_CONSTANT,
	OP_ADD,
	OP_SUBTRACT,
	OP_MULTIPLY,
	OP_DIVIDE,
	OP_NEGATE,
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