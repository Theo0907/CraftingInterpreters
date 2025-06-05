#pragma once

#include "Token.h"
#include "Expr.hpp"

#include <vector>
#include <memory>

class ParseError : public std::runtime_error
{
public:
	ParseError() : std::runtime_error("Failed to parse something.")
	{}
	ParseError(const std::string& msg) : std::runtime_error(msg)
	{}
};

class Parser
{
protected:
	std::vector<Token>	tokens;
	unsigned			current = 0;

	std::shared_ptr<Expr>	expression();
	std::shared_ptr<Expr>	equality();
	std::shared_ptr<Expr>	comparaison();
	std::shared_ptr<Expr>	term();
	std::shared_ptr<Expr>	factor();
	std::shared_ptr<Expr>	unary();
	std::shared_ptr<Expr>	primary();

	bool					check(Token::TokenType type);
	Token					advance();
	bool					isAtEnd();
	Token					peek();
	Token					previous();
	Token					consume(Token::TokenType type, const std::string& message);
	ParseError				error(Token token, const std::string& message);
	void					synchronize();
	bool					match(std::initializer_list<Token::TokenType> types)
	{
		for (auto& it : types)
		{
			if (check(it))
			{
				advance();
				return true;
			}
		}
		return false;
	}
public:
	Parser(const std::vector<Token>& tokens);

	std::shared_ptr<Expr>	parse();
};

