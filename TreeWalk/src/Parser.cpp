#include "Parser.h"

#include "TreeWalk.h"

std::shared_ptr<Expr> Parser::expression()
{
	return equality();
}

std::shared_ptr<Expr> Parser::equality()
{
	std::shared_ptr<Expr> expr = comparaison();

	while (match({ Token::TokenType::BANG_EQUAL, Token::TokenType::EQUAL_EQUAL }))
	{
		std::shared_ptr<Token> op = std::make_shared<Token>(previous());
		std::shared_ptr<Expr> right = comparaison();
		expr = make_shared<Binary>(expr, op, right);
	}

	return expr;
}

std::shared_ptr<Expr> Parser::comparaison()
{
	std::shared_ptr<Expr> expr = term();

	while (match({ Token::TokenType::GREATER, Token::TokenType::GREATER_EQUAL, Token::TokenType::LESS, Token::TokenType::LESS_EQUAL }))
	{
		std::shared_ptr<Token> op = std::make_shared<Token>(previous());
		std::shared_ptr<Expr> right = term();
		expr = make_shared<Binary>(expr, op, right);
	}

	return expr;
}

std::shared_ptr<Expr> Parser::term()
{
	std::shared_ptr<Expr> expr = factor();

	while (match({ Token::TokenType::MINUS, Token::TokenType::PLUS }))
	{
		std::shared_ptr<Token> op = std::make_shared<Token>(previous());
		std::shared_ptr<Expr> right = factor();
		expr = make_shared<Binary>(expr, op, right);
	}

	return expr;
}

std::shared_ptr<Expr> Parser::factor()
{
	std::shared_ptr<Expr> expr = unary();

	while (match({ Token::TokenType::SLASH, Token::TokenType::STAR }))
	{
		std::shared_ptr<Token> op = std::make_shared<Token>(previous());
		std::shared_ptr<Expr> right = unary();
		expr = make_shared<Binary>(expr, op, right);
	}

	return expr;
}

std::shared_ptr<Expr> Parser::unary()
{
	if (match({ Token::TokenType::BANG, Token::TokenType::MINUS }))
	{
		std::shared_ptr<Token> op = std::make_shared<Token>(previous());
		std::shared_ptr<Expr> right = unary();
		return make_shared<Unary>(op, right);
	}
	
	return primary();
}

std::shared_ptr<Expr> Parser::primary()
{
	if (match({ Token::TokenType::FALSE })) return std::make_shared<Literal>(std::make_shared<Object>(false));
	if (match({ Token::TokenType::TRUE })) return std::make_shared<Literal>(std::make_shared<Object>(true));
	if (match({ Token::TokenType::NIL })) return std::make_shared<Literal>(std::make_shared<Object>());

	if (match({ Token::TokenType::NUMBER, Token::TokenType::STRING }))
		return std::make_shared<Literal>(std::make_shared<Object>(previous().literal));

	if (match({ Token::TokenType::LEFT_PAREN }))
	{
		std::shared_ptr<Expr> expr = expression();
		consume(Token::TokenType::RIGHT_PAREN, "Expect ')' after expression.");
		return std::make_shared<Grouping>(expr);
	}

	throw error(peek(), "Expect expression.");
}

bool Parser::check(Token::TokenType type)
{
	if (isAtEnd()) 
		return false;
	return peek().type == type;
}

Token Parser::advance()
{
	if (!isAtEnd()) 
		current++;
	return previous();
}

bool Parser::isAtEnd()
{
	return peek().type == Token::TokenType::END_OF_FILE;
}

Token Parser::peek()
{
	return tokens[current];
}

Token Parser::previous()
{
	return tokens[current - 1];
}

Token Parser::consume(Token::TokenType type, const std::string& message)
{
	if (check({ type }))
		return advance();
	throw error(peek(), message);
}

ParseError Parser::error(Token token, const std::string& message)
{
	TreeWalk::Error(token, message);
	return ParseError(message);
}

void Parser::synchronize()
{
	advance();

	while (!isAtEnd())
	{
		if (previous().type == Token::TokenType::SEMICOLON)
			return;

		switch (peek().type)
		{
		case Token::TokenType::CLASS:
		case Token::TokenType::FUN:
		case Token::TokenType::VAR:
		case Token::TokenType::FOR:
		case Token::TokenType::IF:
		case Token::TokenType::WHILE:
		case Token::TokenType::PRINT:
		case Token::TokenType::RETURN:
			return;
		}

		advance();
	}
}

Parser::Parser(const std::vector<Token>& tokens) : tokens{tokens}
{
}

std::shared_ptr<Expr> Parser::parse()
{
	try
	{
		return expression();
	}
	catch (ParseError error)
	{
		return nullptr;
	}
}
