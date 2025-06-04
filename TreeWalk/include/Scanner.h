#pragma once

#include "Token.h"

#include <vector>
#include <string>

class Scanner
{
private:
	std::string source;
	int start = 0, current = 0, line = 1;

	bool isAtEnd() const;
	void scanToken(std::vector<Token>& result);

	char	advance();
	bool	match(char expected);
	char	peek();
	char	peekNext();
	void	string(std::vector<Token>& result);
	void	number(std::vector<Token>& result);
	void	identifier(std::vector<Token>& result);
	void	addToken(std::vector<Token>& result, Token::TokenType type);
	void	addToken(std::vector<Token>& result, Token::TokenType type, Object literal);
public:
	Scanner(std::string source);
	std::vector<Token> ScanTokens();
};

