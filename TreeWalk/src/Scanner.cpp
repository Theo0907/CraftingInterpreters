#include "Scanner.h"
#include "TreeWalk.h"

#include <unordered_map>

bool Scanner::isAtEnd() const
{
	return current >= source.length();
}

void Scanner::scanToken(std::vector<Token>& result)
{
	char c = advance();
	switch (c)
	{
	case '(': addToken(result, Token::TokenType::LEFT_PAREN); break;
	case ')': addToken(result, Token::TokenType::RIGHT_PAREN); break;
	case '{': addToken(result, Token::TokenType::LEFT_BRACE); break;
	case '}': addToken(result, Token::TokenType::RIGHT_BRACE); break;
	case ',': addToken(result, Token::TokenType::COMMA); break;
	case '.': addToken(result, Token::TokenType::DOT); break;
	case '-': addToken(result, Token::TokenType::MINUS); break;
	case '+': addToken(result, Token::TokenType::PLUS); break;
	case ';': addToken(result, Token::TokenType::SEMICOLON); break;
	case '*': addToken(result, Token::TokenType::STAR); break;
	case '!': addToken(result, match('=') ? Token::TokenType::BANG_EQUAL : Token::TokenType::BANG); break;
	case '=': addToken(result, match('=') ? Token::TokenType::EQUAL_EQUAL : Token::TokenType::EQUAL); break;
	case '<': addToken(result, match('=') ? Token::TokenType::LESS_EQUAL : Token::TokenType::LESS); break;
	case '>': addToken(result, match('=') ? Token::TokenType::GREATER_EQUAL : Token::TokenType::GREATER); break;
	case '/': 
		if (match('/'))
		{
			// A comment goes until the end of the line
			while (peek() != '\n' && !isAtEnd())
				advance();
		}
		else
			addToken(result, Token::TokenType::SLASH);
		break;

	case '\n':
		line++;
		[[fallthrough]];
	case ' ':
	case '\r':
	case '\t':
		// ignore whitespace
		break;
	case '"': string(result); break;
	default: 
		if (std::isdigit(c))
			number(result);
		else if (std::isalpha(c))
			identifier(result);
		else
			TreeWalk::Error(line, "Unexpected character: " + c); break;
	}
}

char Scanner::advance()
{
	return source.at(current++);
}

bool Scanner::match(char expected)
{
	if (isAtEnd() || source.at(current) != expected)
		return false;
	current++;
	return true;
}

char Scanner::peek()
{
	if (isAtEnd()) 
		return '\0';
	return source.at(current);
}

char Scanner::peekNext()
{
	if (current + 1 >= source.length()) 
		return '\0';
	return source.at(current + 1);
}

void Scanner::string(std::vector<Token>& result)
{
	while (peek() != '"' && !isAtEnd())
	{
		if (peek() == '\n') 
			line++;
		advance();
	}

	if (isAtEnd())
	{
		TreeWalk::Error(line, "Unterminated string.");
		return;
	}

	advance();

	std::string value = source.substr(start + 1, current - start - 1);
	Object o{value};
	addToken(result, Token::TokenType::STRING, value);
}

void Scanner::number(std::vector<Token>& result)
{
	while (std::isdigit(peek())) advance();

	// Look for a fractional part
	if (peek() == '.' && std::isdigit(peekNext()))
	{
		advance();

		while (std::isdigit(peek())) advance();
	}

	addToken(result, Token::TokenType::NUMBER, std::atof(source.substr(start, current - start).c_str()));
}

const std::unordered_map<std::string, Token::TokenType> keywords
{
	{"and", Token::TokenType::AND},
	{"class", Token::TokenType::CLASS},
	{"else", Token::TokenType::ELSE},
	{"false", Token::TokenType::FALSE},
	{"for", Token::TokenType::FOR},
	{"fun", Token::TokenType::FUN},
	{"if", Token::TokenType::IF},
	{"nil", Token::TokenType::NIL},
	{"or", Token::TokenType::OR},
	{"print", Token::TokenType::PRINT},
	{"return", Token::TokenType::RETURN},
	{"super", Token::TokenType::SUPER},
	{"this", Token::TokenType::THIS},
	{"true", Token::TokenType::TRUE},
	{"var", Token::TokenType::VAR},
	{"while", Token::TokenType::WHILE},
};

void Scanner::identifier(std::vector<Token>& result)
{
	while (std::isalnum(peek())) advance();

	std::string text = source.substr(start, current - start);
	Token::TokenType type;
	auto find = keywords.find(text);
	if (find != keywords.end())
		type = find->second;
	else
		type = Token::TokenType::IDENTIFIER;

	addToken(result, type);
}

void Scanner::addToken(std::vector<Token>& result, Token::TokenType type)
{
	addToken(result, type, {});
}

void Scanner::addToken(std::vector<Token>& result, Token::TokenType type, Object literal)
{
	std::string text{ source.substr(start, current - start) };
	result.push_back({ type, text, literal, line });
}

Scanner::Scanner(std::string source) : source {source}
{
}

std::vector<Token> Scanner::ScanTokens()
{
	std::vector<Token> result;

	while (!isAtEnd())
	{
		start = current;
		scanToken(result);
	}

	result.push_back(Token(Token::TokenType::END_OF_FILE, "", {}, line));

	return result;
}
