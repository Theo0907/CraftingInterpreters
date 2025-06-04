#include "Token.h"

Token::Token(TokenType type, std::string lexeme, Object literal, int line) : lexeme{lexeme}, literal{literal}, line{line}, type{type}
{
}

Token::operator std::string() const
{
	std::string result;
	std::string litAsString;
	switch (type)
	{
	case (TokenType::LEFT_PAREN):
		result = '('; break;
	case (TokenType::RIGHT_PAREN):
		result = ')'; break;
	case (TokenType::LEFT_BRACE):
		result = '['; break;
	case (TokenType::RIGHT_BRACE):
		result = ']'; break;
	case (TokenType::COMMA):
		result = ','; break;
	case (TokenType::DOT):
		result = '.'; break;
	case (TokenType::MINUS):
		result = '-'; break;
	case (TokenType::PLUS):
		result = '+'; break;
	case (TokenType::SEMICOLON):
		result = ';'; break;
	case (TokenType::SLASH):
		result = '/'; break;
	case (TokenType::STAR):
		result = '*'; break;
	case (TokenType::BANG):
		result = "BANG"; break;
	case (TokenType::BANG_EQUAL):
		result = "BANG_EQUAL"; break;
	case (TokenType::EQUAL):
		result = "EQUAL"; break;
	case (TokenType::EQUAL_EQUAL):
		result = "EQUAL_EQUAL"; break;
	case (TokenType::GREATER):
		result = "GREATER"; break;
	case (TokenType::GREATER_EQUAL):
		result = "GREATER_EQUAL"; break;
	case (TokenType::LESS):
		result = "LESS"; break;
	case (TokenType::LESS_EQUAL):
		result = "LESS_EQUAL"; break;
	case (TokenType::IDENTIFIER):
		result = "IDENTIFIER"; 
		litAsString = literal.GetString();
		break;
	case (TokenType::STRING):
		result = "STRING"; 
		litAsString = literal.GetString();
		break;
	case (TokenType::NUMBER):
		result = "NUMBER"; 
		litAsString = std::to_string(literal.GetNumber());
		break;
	case (TokenType::AND):
		result = "AND"; break;
	case (TokenType::CLASS):
		result = "CLASS"; break;
	case (TokenType::ELSE):
		result = "ELSE"; break;
	case (TokenType::FALSE):
		result = "FALSE"; break;
	case (TokenType::FUN):
		result = "FUN"; break;
	case (TokenType::FOR):
		result = "FOR"; break;
	case (TokenType::IF):
		result = "IF"; break;
	case (TokenType::NIL):
		result = "NIL"; break;
	case (TokenType::OR):
		result = "OR"; break;
	case (TokenType::PRINT):
		result = "PRINT"; break;
	case (TokenType::RETURN):
		result = "RETURN"; break;
	case (TokenType::SUPER):
		result = "SUPER"; break;
	case (TokenType::THIS):
		result = "THIS"; break;
	case (TokenType::TRUE):
		result = "TRUE"; break;
	case (TokenType::VAR):
		result = "VAR"; break;
	case (TokenType::WHILE):
		result = "WHILE"; break;
	case (TokenType::END_OF_FILE):
		result = "END_OF_FILE"; break;
	default:
		result = "WARNING: Token is not defined: " + std::to_string((unsigned char)type);; break;
	}

	return result + " " + lexeme + " " + litAsString;
}
