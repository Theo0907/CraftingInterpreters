#pragma once

#include "Interpreter.h"

#include <string>

class TreeWalk
{
public:
	static void	RunPrompt();
	static void	RunFile(char filePath[]);
	static void Error(int line, const std::string& message);
	static void Error(const class Token& token, const std::string& message);
	static void RuntimeError(const std::string& message);
private:
	static void Report(int line, const std::string& where, const std::string& message);
	static bool hadError;
	static bool hadRuntimeError;
	void Run(const std::string& source);

	static Interpreter interpreter;
};