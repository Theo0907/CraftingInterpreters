#pragma once

#include "common.h"
#include "chunk.h"

#include <string>

bool compile(VM* vm, const std::string& source, Chunk* chunk);