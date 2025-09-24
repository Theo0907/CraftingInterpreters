#pragma once

#include "common.h"
#include "chunk.h"

#include <string>

struct ObjFunction* compile(struct VM* vm, const std::string& source);