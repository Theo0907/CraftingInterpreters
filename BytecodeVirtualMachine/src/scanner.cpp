#include "scanner.h"

Scanner::Scanner(const std::string_view& inSource) : source {inSource}
{
	current = source.begin();
}

bool Scanner::isAtEnd() const
{
	return current == source.end();
}
char Scanner::advance()
{
	return *(current++);
}
char Scanner::peek() const
{
	return *current;
}
char Scanner::peekNext() const
{
	if (isAtEnd())
		return '\0';
	return *(current + 1);
}

bool Scanner::match(char expected)
{
	if (isAtEnd())
		return false;
	if (*current != expected)
		return false;
	++current;
	return true;
}

TokenType Scanner::checkKeyword(int start, int len, const char* rest, TokenType type)
{
	if ((current - source.begin()) == (start + len) && memcmp(source.data() + start, rest, len) == 0)
		return type;
	return TOKEN_IDENTIFIER;
}

void Scanner::skipWhitespace()
{
	while (!isAtEnd())
	{
		char c = peek();
		switch (c)
		{
		case ' ':
		case '\r':
		case '\t':
			advance();
			break;
		case '\n':
			++line;
			advance();
			break;
		case '/':
			if (peekNext() == '/')
			{
				while (peek() != '\n' && !isAtEnd())
					advance();
			}
			else
				return;
			break; // Break could also be put after the while loop. 
		default:
			return;
		}
	}
}

Token Scanner::string()
{
	while (!isAtEnd() && peek() != '"')
	{
		if (peek() == '\n')
			++line;
		advance();
	}

	if (isAtEnd())
		return errorToken("Unterminated string");

	advance();
	return makeToken(TOKEN_STRING);
}

Token Scanner::number()
{
	while (isdigit(peek()))
		advance();

	if (peek() == '.' && isdigit(peekNext()))
	{
		advance();

		while (isdigit(peek()))
			advance();
	}
	return makeToken(TOKEN_NUMBER);
}

TokenType Scanner::identifierType()
{
	switch (source[0])
	{
	case 'a': return checkKeyword(1, 2, "nd", TOKEN_AND);
	case 'c': return checkKeyword(1, 4, "lass", TOKEN_CLASS);
	case 'e': return checkKeyword(1, 3, "lse", TOKEN_ELSE);
	case 'f':
		if (current - source.begin() > 1) {
			switch (source[1]) {
			case 'a': return checkKeyword(2, 3, "lse", TOKEN_FALSE);
			case 'o': return checkKeyword(2, 1, "r", TOKEN_FOR);
			case 'u': return checkKeyword(2, 1, "n", TOKEN_FUN);
			}
		}
		break;
	case 'i': return checkKeyword(1, 1, "f", TOKEN_IF);
	case 'n': return checkKeyword(1, 2, "il", TOKEN_NIL);
	case 'o': return checkKeyword(1, 1, "r", TOKEN_OR);
	case 'p': return checkKeyword(1, 4, "rint", TOKEN_PRINT);
	case 'r': return checkKeyword(1, 5, "eturn", TOKEN_RETURN);
	case 's': return checkKeyword(1, 4, "uper", TOKEN_SUPER);
	case 't':
		if (current - source.begin() > 1) {
			switch (source[1]) {
			case 'h': return checkKeyword(2, 2, "is", TOKEN_THIS);
			case 'r': return checkKeyword(2, 2, "ue", TOKEN_TRUE);
			}
		}
		break;
	case 'v': return checkKeyword(1, 2, "ar", TOKEN_VAR);
	case 'w': return checkKeyword(1, 4, "hile", TOKEN_WHILE);
	}
	return TOKEN_IDENTIFIER;
}

Token Scanner::identifier()
{
	while (std::isalnum(peek()))
		advance();
	return makeToken(identifierType());
}

Token Scanner::makeToken(TokenType type)
{
	Token token;
	token.type = type;
	token.lexeme = std::string_view(source.begin(), current);
	token.line = line;

	return token;
}

Token Scanner::errorToken(const char* message)
{
	Token token;
	token.type = TOKEN_ERROR;
	token.lexeme = std::string_view(message);
	token.line = line;

	return token;
}

Token Scanner::scanToken()
{
	skipWhitespace();
	source = std::string_view(current, source.end());
	current = source.begin();

	if (isAtEnd())
		return makeToken(TOKEN_EOF);

	char c = advance();
	if (std::isdigit(c))
		return number();
	if (std::isalpha(c))
		return identifier();

	switch (c)
	{
	case '(': return makeToken(TOKEN_LEFT_PAREN);
	case ')': return makeToken(TOKEN_RIGHT_PAREN);
	case '{': return makeToken(TOKEN_LEFT_BRACE);
	case '}': return makeToken(TOKEN_RIGHT_BRACE);
	case ';': return makeToken(TOKEN_SEMICOLON);
	case ',': return makeToken(TOKEN_COMMA);
	case '.': return makeToken(TOKEN_DOT);
	case '-': return makeToken(TOKEN_MINUS);
	case '+': return makeToken(TOKEN_PLUS);
	case '/': return makeToken(TOKEN_SLASH);
	case '*': return makeToken(TOKEN_STAR);
	case '!': return makeToken(match('=') ? TOKEN_BANG_EQUAL : TOKEN_BANG);
	case '=': return makeToken(match('=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL);
	case '<': return makeToken(match('=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
	case '>': return makeToken(match('=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);
	case '"': return string();
	}

	return errorToken("Unexpected character.");
}
