#pragma once

#include <cstdint>
#include <cstddef>

constexpr size_t UINT8_COUNT = UINT8_MAX + 1;

// Maybe add a profile for this in project config?
#ifdef _DEBUG
#define DEBUG_PRINT_CODE
#define DEBUG_TRACE_EXECUTION
#endif