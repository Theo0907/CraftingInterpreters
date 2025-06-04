#pragma once

#include <string>

class TreeWalk
{
public:
	static void	RunPrompt();
	static void	RunFile(char filePath[]);
	static void Error(int line, const std::string& message);
private:
	static void Report(int line, const std::string& where, const std::string& message);
	static bool hadError;
	void Run(const std::string& source);
};