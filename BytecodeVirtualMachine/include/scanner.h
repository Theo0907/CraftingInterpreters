#pragma once

#include <string>
#include <string_view>

// This could become an enum class to remove the TOKEN_ prefix without polluting global namespace too much
enum TokenType {
	// Single-character tokens.
	TOKEN_LEFT_PAREN, TOKEN_RIGHT_PAREN,
	TOKEN_LEFT_BRACE, TOKEN_RIGHT_BRACE,
	TOKEN_COMMA, TOKEN_DOT, TOKEN_MINUS, TOKEN_PLUS,
	TOKEN_SEMICOLON, TOKEN_SLASH, TOKEN_STAR,
	// One or two character tokens.
	TOKEN_BANG, TOKEN_BANG_EQUAL,
	TOKEN_EQUAL, TOKEN_EQUAL_EQUAL,
	TOKEN_GREATER, TOKEN_GREATER_EQUAL,
	TOKEN_LESS, TOKEN_LESS_EQUAL,
	// Literals.
	TOKEN_IDENTIFIER, TOKEN_STRING, TOKEN_NUMBER,
	// Keywords.
	TOKEN_AND, TOKEN_CLASS, TOKEN_ELSE, TOKEN_FALSE,
	TOKEN_FOR, TOKEN_FUN, TOKEN_IF, TOKEN_NIL, TOKEN_OR,
	TOKEN_PRINT, TOKEN_RETURN, TOKEN_SUPER, TOKEN_THIS,
	TOKEN_TRUE, TOKEN_VAR, TOKEN_WHILE,

	TOKEN_ERROR, TOKEN_EOF,

	TOKEN_COUNT
};

struct Token
{
	TokenType type;
	std::string_view lexeme;
	int line;
};

struct Scanner
{
	std::string_view source;
	std::string_view::iterator current;
	int line = 1;
	Scanner(const std::string_view& source);

	Token	scanToken();

protected:
	Token	makeToken(TokenType type);
	Token	errorToken(const char* message);
	Token	string();
	Token	number();
	Token	identifier();
	TokenType identifierType();
	
public:
	bool	isAtEnd() const;
protected:
	char	advance();
	char	peek() const;
	char	peekNext() const;
	bool	match(char expected);
	TokenType	checkKeyword(int start, int len, const char* rest, TokenType type);
	void	skipWhitespace();
};