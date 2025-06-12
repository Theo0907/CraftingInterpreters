#include "compiler.h"

#include "scanner.h"

#include <iostream>
#include <format>
#include <string_view>

void compile(const std::string& source)
{
	Scanner scanner{ source };

	int line = -1;
	while (true)
	{
		Token token = scanner.scanToken();
		if (token.line != line)
		{
			std::cout << std::format("{:4d} ", token.line);
			line = token.line;
		}
		else
			std::cout << "   | ";

		std::cout << std::format("{:2d} '{}'", (int)token.type, token.lexeme) << std::endl;

		if (token.type == TOKEN_EOF)
			break;
	}
}
