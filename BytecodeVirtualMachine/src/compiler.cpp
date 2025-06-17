#include "compiler.h"

#include "scanner.h"

#ifdef DEBUG_PRINT_CODE
#include "debug.h"
#endif

#include <iostream>
#include <format>
#include <string_view>
#include <functional>

static void errorAt(class Parser& parser, Token& token, const char* message);

static void errorAtCurrent(class Parser& parser, const char* message);
static void error(class Parser& parser, const char* message);

enum Precedence
{
	PREC_NONE,
	PREC_ASSIGNMENT,  // =
	PREC_OR,          // or
	PREC_AND,         // and
	PREC_EQUALITY,    // == !=
	PREC_COMPARISON,  // < > <= >=
	PREC_TERM,        // + -
	PREC_FACTOR,      // * /
	PREC_UNARY,       // ! -
	PREC_CALL,        // . ()
	PREC_PRIMARY
};

using ParseFn = std::function<void(class Parser&)>;

struct ParseRule
{
	ParseFn prefix;
	ParseFn infix;
	Precedence precedence;
};

class Parser
{
public:
	Token	current;
	Token	previous;
	bool	hadError = false;
	bool	panicMode = false; // TODO: Maybe replace by throw and try catch 

	Scanner& scanner;
	class Compiler& compiler;

	Parser(Scanner& scanner, class Compiler& compiler) : scanner { scanner }, compiler{ compiler }
	{
		InitRules();
	}

	void	advance()
	{
		previous = current;
		while (true)
		{
			current = scanner.scanToken();
			if (current.type != TOKEN_ERROR)
				break;

			errorAtCurrent(*this, current.lexeme.data());
		}
	}

	void	consume(TokenType type, const char* message)
	{
		if (current.type == type)
		{
			advance();
			return;
		}

		errorAtCurrent(*this, message);
	}

	void	expression();

	void	number();
	void	grouping();
	void	unary();
	void	binary();
	void	literal();
	void	parsePrecedence(Precedence precedence);

	ParseRule rules[TOKEN_COUNT];
	void	InitRules(); 
	ParseRule* getRule(TokenType type)
	{
		return &rules[type];
	}
};

class Compiler
{
public:
	Scanner scanner;
	Parser parser;

	Chunk* compilingChunk;

	// Maybe move what is done in this constructor to a compile func that can be called on an instance of the class, instead of the current static compile function?
	Compiler(const std::string& source, Chunk* chunk) : scanner{ source }, parser{ scanner, *this }, compilingChunk{ chunk }
	{}

	Chunk* currentChunk()
	{
		return compilingChunk;
	}

	// All of this bellow should probably be in the parser rather than the compiler as this is where it will be used

	uint8_t	makeConstant(Value value)
	{
		int constant = currentChunk()->addConstant(value);
		if (constant > UINT8_MAX)
		{
			// If this is hit, need to add another instruction to add more constants. This should not be needed honestly, as this is not an actual production language.
			error(parser, "Too many constants in one chunk.");
			return 0;
		}

		return static_cast<uint8_t>(constant);
	}

	void	endCompiler();
	void	emitReturn();
	void	emitConstant(Value value);

	void	emitByte(uint8_t byte);
	void	emitBytes(uint8_t byte1, uint8_t byte2);
};

static void errorAt(class Parser& parser, Token& token, const char* message)
{
	if (parser.panicMode)
		return;
	parser.panicMode = true; // This should be replaced by a throw and try catch
	std::cerr << std::format("[line {}] Error", token.line);

	if (token.type == TOKEN_EOF)
		std::cerr << " at end";
	else if (token.type == TOKEN_ERROR)
	{
		// Nothing, should have been handled previously
	}
	else
		std::cerr << std::format(" at '{}'", token.lexeme);
	std::cerr << std::format(": {}", message) << std::endl;
	parser.hadError = true;
}
static void errorAtCurrent(class Parser& parser, const char* message)
{
	errorAt(parser, parser.current, message);
}
static void error(class Parser& parser, const char* message)
{
	errorAt(parser, parser.previous, message);
}


bool compile(const std::string& source, Chunk* chunk)
{
	Compiler compiler{ source, chunk };
	compiler.parser.advance();
	compiler.parser.expression();
	compiler.parser.consume(TOKEN_EOF, "Expect end of expression");
	compiler.endCompiler();

	return !compiler.parser.hadError;
}

void Compiler::endCompiler()
{
	emitReturn();
#ifdef DEBUG_PRINT_CODE
	if (!parser.hadError)
		disassembleChunk(currentChunk(), "code");
#endif
}

void Compiler::emitReturn()
{
	emitByte(OP_RETURN);
}

void Compiler::emitConstant(Value value)
{
	emitBytes(OP_CONSTANT, makeConstant(value));
}

void Compiler::emitByte(uint8_t byte)
{
	currentChunk()->write(byte, parser.previous.line);
}

void Compiler::emitBytes(uint8_t byte1, uint8_t byte2)
{
	emitByte(byte1);
	emitByte(byte2);
}

void Parser::expression()
{
	parsePrecedence(PREC_ASSIGNMENT);
}

void Parser::number()
{
	double value = std::strtod(previous.lexeme.data(), nullptr);
	compiler.emitConstant(value);
}

void Parser::grouping()
{
	expression();
	consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
}
void Parser::unary()
{
	TokenType opType = previous.type;

	// Compile the operand
	parsePrecedence(PREC_UNARY);
	
	// Emit the operator instruction
	switch (opType)
	{
	case TOKEN_BANG: 
		compiler.emitByte(OP_NOT);
		break;
	case TOKEN_MINUS:
		compiler.emitByte(OP_NEGATE);
		break;
	default:return; // Unreachable
	}
}

void Parser::binary()
{
	TokenType operatorType = previous.type;
	ParseRule* rule = getRule(operatorType);
	parsePrecedence((Precedence)(rule->precedence + 1));

	switch (operatorType) 
	{
	case TOKEN_BANG_EQUAL:    compiler.emitBytes(OP_EQUAL, OP_NOT); break;
	case TOKEN_EQUAL_EQUAL:   compiler.emitByte(OP_EQUAL); break;
	case TOKEN_GREATER:       compiler.emitByte(OP_GREATER); break;
	case TOKEN_GREATER_EQUAL: compiler.emitBytes(OP_LESS, OP_NOT); break;
	case TOKEN_LESS:          compiler.emitByte(OP_LESS); break;
	case TOKEN_LESS_EQUAL:    compiler.emitBytes(OP_GREATER, OP_NOT); break;
	case TOKEN_PLUS:          compiler.emitByte(OP_ADD); break;
	case TOKEN_MINUS:         compiler.emitByte(OP_SUBTRACT); break;
	case TOKEN_STAR:          compiler.emitByte(OP_MULTIPLY); break;
	case TOKEN_SLASH:         compiler.emitByte(OP_DIVIDE); break;
	default: return; // Unreachable.
	}
}

void Parser::literal()
{
	switch (previous.type)
	{
	case TOKEN_FALSE: compiler.emitByte(OP_FALSE); break;
	case TOKEN_NIL: compiler.emitByte(OP_NIL); break;
	case TOKEN_TRUE: compiler.emitByte(OP_TRUE); break;
	default: return; // Unreachable.
	}
}

void Parser::parsePrecedence(Precedence precedence)
{
	advance();
	ParseFn prefixRule = getRule(previous.type)->prefix;
	if (!prefixRule)
	{
		error(*this, "Expect expression.");
		return;
	}

	prefixRule(*this);

	while (precedence <= getRule(current.type)->precedence)
	{
		advance();
		ParseFn infixRule = getRule(previous.type)->infix;
		infixRule(*this);
	}
}

void Parser::InitRules()
{
	ParseRule tempRules[TOKEN_COUNT]{
 {&Parser::grouping, NULL,   PREC_NONE},// [TOKEN_LEFT_PAREN]
 {NULL,     NULL,   PREC_NONE},// [TOKEN_RIGHT_PAREN]
 {NULL,     NULL,   PREC_NONE},// [TOKEN_LEFT_BRACE]
 {NULL,     NULL,   PREC_NONE},// [TOKEN_RIGHT_BRACE]
 {NULL,     NULL,   PREC_NONE},// [TOKEN_COMMA]
 {NULL,     NULL,   PREC_NONE},// [TOKEN_DOT]
 {&Parser::unary,   &Parser::binary, PREC_TERM},// [TOKEN_MINUS]
 {NULL,     &Parser::binary, PREC_TERM},// [TOKEN_PLUS]
 {NULL,     NULL,   PREC_NONE},// [TOKEN_SEMICOLON]
 {NULL,     &Parser::binary, PREC_FACTOR},// [TOKEN_SLASH]
 {NULL,     &Parser::binary, PREC_FACTOR},// [TOKEN_STAR]
 {&Parser::unary,     NULL,   PREC_NONE},// [TOKEN_BANG]
 {NULL,     &Parser::binary,   PREC_EQUALITY},// [TOKEN_BANG_EQUAL]
 {NULL,     NULL,   PREC_NONE},// [TOKEN_EQUAL]
 {NULL,     &Parser::binary,   PREC_EQUALITY},// [TOKEN_EQUAL_EQUAL]
 {NULL,     &Parser::binary,   PREC_COMPARISON},// [TOKEN_GREATER]
 {NULL,     &Parser::binary,   PREC_COMPARISON},// [TOKEN_GREATER_EQUAL]
 {NULL,     &Parser::binary,   PREC_COMPARISON},// [TOKEN_LESS]
 {NULL,     &Parser::binary,   PREC_COMPARISON},// [TOKEN_LESS_EQUAL]
 {NULL,     NULL,   PREC_NONE},// [TOKEN_IDENTIFIER]
 {NULL,     NULL,   PREC_NONE},// [TOKEN_STRING]
 {&Parser::number,   NULL,   PREC_NONE},// [TOKEN_NUMBER]
 {NULL,     NULL,   PREC_NONE},// [TOKEN_AND]
 {NULL,     NULL,   PREC_NONE},// [TOKEN_CLASS]
 {NULL,     NULL,   PREC_NONE},// [TOKEN_ELSE]
 {&Parser::literal,     NULL,   PREC_NONE},// [TOKEN_FALSE]
 {NULL,     NULL,   PREC_NONE},// [TOKEN_FOR]
 {NULL,     NULL,   PREC_NONE},// [TOKEN_FUN]
 {NULL,     NULL,   PREC_NONE},// [TOKEN_IF]
 {&Parser::literal,     NULL,   PREC_NONE},// [TOKEN_NIL]
 {NULL,     NULL,   PREC_NONE},// [TOKEN_OR]
 {NULL,     NULL,   PREC_NONE},// [TOKEN_PRINT]
 {NULL,     NULL,   PREC_NONE},// [TOKEN_RETURN]
 {NULL,     NULL,   PREC_NONE},// [TOKEN_SUPER]
 {NULL,     NULL,   PREC_NONE},// [TOKEN_THIS]
 {&Parser::literal,     NULL,   PREC_NONE},// [TOKEN_TRUE]
 {NULL,     NULL,   PREC_NONE},// [TOKEN_VAR]
 {NULL,     NULL,   PREC_NONE},// [TOKEN_WHILE]
 {NULL,     NULL,   PREC_NONE},// [TOKEN_ERROR]
 {NULL,     NULL,   PREC_NONE},// [TOKEN_EOF]
	}; 
	for (int i = 0; i < TOKEN_COUNT; ++i)
		rules[i] = tempRules[i];
}
