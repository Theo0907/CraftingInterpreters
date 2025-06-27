#include "debug.h"

#include <iostream>
#include <format>
#include <ios>
#include <iomanip>

void disassembleChunk(Chunk* chunk, const std::string& name)
{
	std::cout << std::format("== {} ==", name) << std::endl;

	for (int offset = 0; offset < chunk->code.size();)
		offset = disassembleInstruction(chunk, offset);
}

static int simpleInstruction(const std::string& name, int offset) {
	std::cout << name << std::endl;
	return offset + 1;
}

static int constantInstruction(const std::string& name, Chunk* chunk,
	int offset) {
	uint8_t constant = chunk->code[offset + 1];
	//std::cout << std::setfill(' ') << std::setw(16) << std::left << name << " ";
	//std::cout << std::right << std::format("{:*>4d} ", constant);
	std::cout << std::format("{:<16s} {:04d} ", name, constant);
	//printf("%-16s %4d '", name, constant);
	printValue(chunk->constants[constant]);
	std::cout << std::endl;

	return offset + 2;
}

int disassembleInstruction(Chunk* chunk, int offset)
{
	std::cout << std::format("{:04} ", offset);
	if (offset > 0 && chunk->lineInfo[offset] == chunk->lineInfo[offset - 1])
		std::cout << "   | ";
	else
		std::cout << std::format("{:4d} ", chunk->lineInfo[offset]);

	uint8_t instruction = chunk->code[offset];
	switch (instruction)
	{
	case OP_CONSTANT:
		return constantInstruction("OP_CONSTANT", chunk, offset);
	case OP_NIL:
		return simpleInstruction("OP_NIL", offset);
	case OP_TRUE:
		return simpleInstruction("OP_TRUE", offset);
	case OP_FALSE:
		return simpleInstruction("OP_FALSE", offset);
	case OP_POP:
		return simpleInstruction("OP_POP", offset);
	case OP_GET_GLOBAL:
		return constantInstruction("OP_GET_GLOBAL", chunk, offset);
	case OP_DEFINE_GLOBAL:
		return constantInstruction("OP_DEFINE_GLOBAL", chunk, offset);
	case OP_SET_GLOBAL:
		return constantInstruction("OP_SET_GLOBAL", chunk, offset);
	case OP_EQUAL:
		return simpleInstruction("OP_EQUAL", offset);
	case OP_GREATER:
		return simpleInstruction("OP_GREATER", offset);
	case OP_LESS:
		return simpleInstruction("OP_LESS", offset);
	case OP_ADD:
		return simpleInstruction("OP_ADD", offset);
	case OP_SUBTRACT:
		return simpleInstruction("OP_SUBTRACT", offset);
	case OP_MULTIPLY:
		return simpleInstruction("OP_MULTIPLY", offset);
	case OP_DIVIDE:
		return simpleInstruction("OP_DIVIDE", offset);
	case OP_NOT:
		return simpleInstruction("OP_NOT", offset);
	case OP_NEGATE:
		return simpleInstruction("OP_NEGATE", offset);
	case OP_PRINT:
		return simpleInstruction("OP_PRINT", offset);
	case OP_RETURN:
		return simpleInstruction("OP_RETURN", offset);
	default:
		std::cout << "Unknown opcode " << instruction << std::endl;
		return offset + 1;
	}
}
