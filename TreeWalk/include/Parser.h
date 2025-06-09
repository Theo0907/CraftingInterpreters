#pragma once

#include "Token.h"
#include "Expr.hpp"
#include "Stmt.hpp"

#include <vector>
#include <list>
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
	std::shared_ptr<Expr>	assignment();
	std::shared_ptr<Expr>	orExpr();
	std::shared_ptr<Expr>	andExpr();
	std::shared_ptr<Expr>	equality();
	std::shared_ptr<Expr>	comparaison();
	std::shared_ptr<Expr>	term();
	std::shared_ptr<Expr>	factor();
	std::shared_ptr<Expr>	unary();
	std::shared_ptr<Expr>	call();
	std::shared_ptr<Expr>	finishCall(std::shared_ptr<Expr> callee);
	std::shared_ptr<Expr>	primary();

	std::shared_ptr<Stmt>	statement();
	std::shared_ptr<Stmt>	printStatement();
	std::shared_ptr<Stmt>	forStatement();
	std::shared_ptr<Stmt>	whileStatement();
	std::shared_ptr<Stmt>	ifStatement();
	std::shared_ptr<Stmt>	expressionStatement();
	std::shared_ptr<Stmt>	declaration();
	std::shared_ptr<Stmt>	varDeclaration();

	std::list<std::shared_ptr<Stmt>> block();

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

	std::list<std::shared_ptr<Stmt>>	parse();
};

