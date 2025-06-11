#include "chunk.h"

Chunk::Chunk()
{
	// Start code array size at 8 as to not constantly re-alloc when starting to fill it up
	code.reserve(8);
	lineInfo.reserve(8);
	constants.reserve(8);
}

void Chunk::write(uint8_t byte, int line)
{
	code.push_back(byte);
	lineInfo.push_back(line);
}

int Chunk::addConstant(Value v)
{
	// TODO: Search beforehand to see if constant already exists
	// TODO: Add another instruction that allows to have more than 256 (a byte) worth of constants (for example: 3 bytes is 16777216 possible constants)
	constants.push_back(v);
	return constants.size() - 1;
}
