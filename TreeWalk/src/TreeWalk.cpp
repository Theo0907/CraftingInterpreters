#include "TreeWalk.h"

#include "Token.h"
#include "Scanner.h"
#include "Parser.h"
#include "AstPrinter.h"

#include <cstddef>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

void TreeWalk::RunPrompt()
{
	TreeWalk tw;
	while (true)
	{
		std::cout << "> ";
		std::string line;
		std::getline(std::cin, line, '\n');
		//std::cin >> line;
		if (line.size() == 0 || line == "exit")
			break;
		tw.Run(line);
		hadError = false;
	}
}

void TreeWalk::RunFile(char filePath[])
{
	std::ifstream input{ filePath };
	std::stringstream source;
	source << input.rdbuf();

	TreeWalk tw;
	tw.Run(source.str());
	if (hadError)
		exit(65);
}

void TreeWalk::Error(int line, const std::string& message)
{
	Report(line, "", message);
}

void TreeWalk::Error(const Token& token, const std::string& message)
{
	if (token.type == Token::TokenType::END_OF_FILE)
		Report(token.line, " at end", message);
	else
		Report(token.line, " at '" + token.lexeme + "'", message);
}

bool TreeWalk::hadError = false;

void TreeWalk::Report(int line, const std::string& where, const std::string& message)
{
	std::cerr << "[line " << std::to_string(line) << "] Error" << where << ": " << message;
	hadError = true;
}

void TreeWalk::Run(const std::string& source)
{
	Scanner scanner{ source };
	std::vector<Token> tokens = scanner.ScanTokens();
	Parser parser{ tokens };
	std::shared_ptr<Expr>	expr = parser.parse();

	// Stop if syntax error
	if (hadError)
		return;

	std::cout << std::get<std::string>(AstPrinter().print(*expr)) << std::endl;
}
