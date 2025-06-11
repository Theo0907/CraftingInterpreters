#pragma once

#include "chunk.h"

#include <string>

void	disassembleChunk(Chunk* chunk, const std::string& name);
int		disassembleInstruction(Chunk* chunk, int offset);