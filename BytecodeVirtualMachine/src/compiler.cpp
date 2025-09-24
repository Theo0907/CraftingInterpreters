#include "compiler.h"

#include "scanner.h"
#include "vm.h"

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

using ParseFn = std::function<void(class Parser&, bool)>;

struct ParseRule
{
	ParseFn prefix;
	ParseFn infix;
	Precedence precedence;
};



struct Local
{
	Token name;
	int depth;
};

struct Locals
{
	Local locals[UINT8_COUNT];
	int localCount = 0;
	int scopeDepth = 0;
};

enum FunctionType
{
	TYPE_FUNCTION,
	TYPE_SCRIPT,
};

class Parser
{
public:
	Token	current;
	Token	previous;
	bool	hadError = false;
	bool	panicMode = false; // TODO: Maybe replace by throw and try catch 

	Scanner scanner;
	class Compiler* compiler;
	VM* vm;

	Parser(VM* vm, const std::string& source, class Compiler* compiler = nullptr) : scanner { source }, compiler{ compiler }, vm{ vm }
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

	void	synchronize();

	void	expression();
	void	declaration();
	void	varDeclaration();
	void	funDeclaration();
	void	statement();
	void	printStatement();
	void	expressionStatement();
	void	ifStatement();
	void	returnStatement();
	void	whileStatement();
	void	forStatement();

	void	beginScope();
	void	endScope();
	void	block();
	void	function(FunctionType type);

	void	number(bool canAssign);
	void	string(bool canAssign);
	void	variable(bool canAssign);
	void	namedVariable(Token name, bool canAssign);
	void	grouping(bool canAssign);
	void	unary(bool canAssign);
	void	binary(bool canAssign);
	void	call(bool canAssign);
	void	literal(bool canAssign);
	void	and_(bool canAssign);
	void	or_(bool canAssign);

	void	parsePrecedence(Precedence precedence);
	uint8_t	parseVariable(const char* errorMessage);
	uint8_t	identifierConstant(Token* name);
	bool	identifierEquals(const Token& a, const Token& b);
	int		resolveLocal(Locals* locals, Token* name);
	void	addLocal(const Token& name);
	void	defineVariable(uint8_t global);
	uint8_t	argumentList();
	void	markInitialized();
	void	declareVariable();

	bool	match(TokenType type);
	bool	check(TokenType type);

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
	Parser& parser;
	VM* vm;
	Compiler* enclosing = nullptr;

	ObjFunction* function = nullptr;
	FunctionType type = FunctionType::TYPE_SCRIPT;

	Locals locals;

	// Maybe move what is done in this constructor to a compile func that can be called on an instance of the class, instead of the current static compile function?
	Compiler(Parser& parser, VM* vm, FunctionType type = FunctionType::TYPE_SCRIPT) : parser{ parser }, vm{ vm }, type{ type }
	{
		enclosing = parser.compiler;
		parser.compiler = this;
		function = newFunction(vm);
		if (type != TYPE_SCRIPT)
		{
			function->name = copyString(static_cast<int>(parser.previous.lexeme.length()), parser.previous.lexeme.data(), vm);
		}

		// Reserving stack slot 0 for the VM
		Local* local = &locals.locals[locals.localCount++];
		local->depth = 0;
		local->name.lexeme = "";
	}

	Chunk* currentChunk()
	{
		return &function->chunk;
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

	ObjFunction*	endCompiler();
	void			emitReturn();
	void			emitConstant(Value value);
	int				emitJump(uint8_t instruction);
	void			patchJump(int offset);
	void			emitLoop(int loopStart);

	void			emitByte(uint8_t byte);
	void			emitBytes(uint8_t byte1, uint8_t byte2);
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


ObjFunction* compile(VM* vm, const std::string& source)
{
	Parser parser{ vm, source };
	Compiler compiler{ parser, vm };
	parser.advance();
	while (!parser.match(TOKEN_EOF))
	{
		parser.declaration();
	}

	ObjFunction* func = compiler.endCompiler();
	return parser.hadError ? nullptr : func;
}

ObjFunction* Compiler::endCompiler()
{
	emitReturn();
#ifdef DEBUG_PRINT_CODE
	if (!parser.hadError)
		disassembleChunk(currentChunk(), func->name != nullptr ? func->name->chars : "<script>");
#endif

	parser.compiler = enclosing;
	return function;
}

void Compiler::emitReturn()
{
	emitByte(OP_NIL);
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

void Parser::synchronize()
{
	panicMode = false;
	
	while (current.type != TOKEN_EOF)
	{
		if (previous.type == TOKEN_SEMICOLON)
			return;
		switch (current.type)
		{
		case TOKEN_CLASS:
		case TOKEN_FUN:
		case TOKEN_VAR:
		case TOKEN_FOR:
		case TOKEN_IF:
		case TOKEN_WHILE:
		case TOKEN_PRINT:
		case TOKEN_RETURN:
			return;

		default:// Next token
			advance();
		}

	}
}

void Parser::expression()
{
	parsePrecedence(PREC_ASSIGNMENT);
}

void Parser::declaration()
{
	if (match(TOKEN_FUN))
		funDeclaration();
	else if (match(TOKEN_VAR))
		varDeclaration();
	else
		statement();

	if (panicMode)
		synchronize();
}

void Parser::varDeclaration()
{
	uint8_t global = parseVariable("Expect variable name.");

	if (match(TOKEN_EQUAL))
		expression();
	else
		compiler->emitByte(OP_NIL);

	consume(TOKEN_SEMICOLON, "Expect ';' after variable declaration");
	defineVariable(global);
}

void Parser::funDeclaration()
{
	uint8_t global = parseVariable("Expect function name.");
	// Mark function variable initialized so that it can be used immediatly after beeing declared
	// to allow for easy recursive functions
	markInitialized();
	function(TYPE_FUNCTION);
	defineVariable(global);
}

void Parser::statement()
{
	if (match(TOKEN_PRINT))
		printStatement();
	else if (match(TOKEN_FOR))
		forStatement();
	else if (match(TOKEN_IF))
		ifStatement();
	else if (match(TOKEN_RETURN))
		returnStatement();
	else if (match(TOKEN_WHILE))
		whileStatement();
	else if (match(TOKEN_LEFT_BRACE))
	{
		beginScope();
		block();
		endScope();
	}
	else
		expressionStatement();
}

void Parser::printStatement()
{
	expression();
	consume(TOKEN_SEMICOLON, "Expect ';' after value.");
	compiler->emitByte(OP_PRINT);
}

void Parser::expressionStatement()
{
	expression();
	consume(TOKEN_SEMICOLON, "Expect ';' after expression.");
	compiler->emitByte(OP_POP);
}

void Parser::ifStatement()
{
	consume(TOKEN_LEFT_PAREN, "Expect '(' after 'if'.");
	expression();
	consume(TOKEN_RIGHT_PAREN, "Expect ')' after condition.");

	// Keep address of the jump operand to patch later with jump destination
	int thenJump = compiler->emitJump(OP_JUMP_IF_FALSE);
	// Clean up the if condition before the statement to keep stack clean in the statement
	compiler->emitByte(OP_POP);
	statement();

	// Prepare a jump to after a potential else jump
	int elseJump = compiler->emitJump(OP_JUMP);

	compiler->patchJump(thenJump);
	// Clean up the if condition in the else branch too
	compiler->emitByte(OP_POP);

	if (match(TOKEN_ELSE))
		statement();
	compiler->patchJump(elseJump);
}

void Parser::returnStatement()
{
	if (compiler->type == TYPE_SCRIPT)
		error(*this, "Can't return from top-level code.");

	if (match(TOKEN_SEMICOLON))
		compiler->emitReturn();
	else
	{
		expression();
		consume(TOKEN_SEMICOLON, "Expect ';' after return value.");
		compiler->emitByte(OP_RETURN);
	}
}

void Parser::whileStatement()
{
	// Get address of first instruction to jump to later
	int loopStart = static_cast<int>(compiler->currentChunk()->code.size());
	consume(TOKEN_LEFT_PAREN, "Expect '(' after 'while'.");
	expression();
	consume(TOKEN_RIGHT_PAREN, "Expect ')' after condition.");

	// while condition evaluation here
	int exitJump = compiler->emitJump(OP_JUMP_IF_FALSE);
	// Clean up the stack in either branch
	compiler->emitByte(OP_POP);
	statement();

	// Jump back to the start to re-evaluate the while condition
	compiler->emitLoop(loopStart);

	compiler->patchJump(exitJump);
	// Clean up the stack in either branch
	compiler->emitByte(OP_POP);
}

void Parser::forStatement()
{
	// Begin scope for potential variable declaration in the for
	beginScope();
	consume(TOKEN_LEFT_PAREN, "Expect '(' after 'for'.");
	if (match(TOKEN_SEMICOLON))
	{
		// No initializer, nothing to do here
	}
	else if (match(TOKEN_VAR))
		varDeclaration();
	else
		expressionStatement();

	// Get address of for condition to loop to
	int loopStart = static_cast<int>(compiler->currentChunk()->code.size());
	int exitJump = -1;
	if (!match(TOKEN_SEMICOLON))
	{
		expression();
		consume(TOKEN_SEMICOLON, "Expect ';' after loop condition.");

		// Jump out of the loop if the condition is false.
		exitJump = compiler->emitJump(OP_JUMP_IF_FALSE);
		compiler->emitByte(OP_POP); // Clear the condition from the stack in every branch
	}

	if (!match(TOKEN_RIGHT_PAREN))
	{
		int bodyJump = compiler->emitJump(OP_JUMP);
		int incrementStart = static_cast<int>(compiler->currentChunk()->code.size());
		expression();
		compiler->emitByte(OP_POP);
		consume(TOKEN_RIGHT_PAREN, "Expect ')' after for clauses.");

		compiler->emitLoop(loopStart);
		loopStart = incrementStart;
		compiler->patchJump(bodyJump);
	}
	statement();
	compiler->emitLoop(loopStart);

	// Patch in exit jump if it exists
	if (exitJump != -1)
	{
		compiler->patchJump(exitJump);
		compiler->emitByte(OP_POP);
	}
	// End the scope after the loop
	endScope();
}

void Parser::block()
{
	while (!check(TOKEN_RIGHT_BRACE) && !check(TOKEN_EOF))
		declaration();

	consume(TOKEN_RIGHT_BRACE, "Expect '}' after block.");
}

void Parser::function(FunctionType type)
{
	Compiler funcCompiler{*this, vm, type};

	// No need to end this scope as we end the compiler
	beginScope();

	consume(TOKEN_LEFT_PAREN, "Expect '(' after function name.");
	if (!check(TOKEN_RIGHT_PAREN))
	{
		do
		{
			compiler->function->arity++;
			if (compiler->function->arity > 255)
				errorAtCurrent(*this, "Can't have more than 255 parameters.");
			uint8_t constant = parseVariable("Expect parameter name.");
			defineVariable(constant);
		} while (match(TOKEN_COMMA));
	}
	consume(TOKEN_RIGHT_PAREN, "Expect ')' after parameters.");
	consume(TOKEN_LEFT_BRACE, "Expect '{' before function body.");
	// Do function body
	block();

	// Does the job of endScope too
	ObjFunction* function = funcCompiler.endCompiler();
	compiler->emitBytes(OP_CONSTANT, compiler->makeConstant({ function }));
}

void Parser::beginScope()
{
	++compiler->locals.scopeDepth;
}

void Parser::endScope()
{
	--compiler->locals.scopeDepth;

	// Remove out of scope variables from locals
	while (compiler->locals.localCount > 0 && compiler->locals.locals[compiler->locals.localCount - 1].depth > compiler->locals.scopeDepth)
	{
		// Pop unused variable from stack
		// TODO: Add a popn instruction to pop n elements from the stack
		compiler->emitByte(OP_POP);
		--compiler->locals.localCount;
	}
}

void Parser::number(bool)
{
	double value = std::strtod(previous.lexeme.data(), nullptr);
	compiler->emitConstant(value);
}

void Parser::string(bool)
{
	compiler->emitConstant({ vm, previous.lexeme.substr(1, previous.lexeme.length() - 2) });
}

void Parser::grouping(bool)
{
	expression();
	consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
}
void Parser::unary(bool)
{
	TokenType opType = previous.type;

	// Compile the operand
	parsePrecedence(PREC_UNARY);
	
	// Emit the operator instruction
	switch (opType)
	{
	case TOKEN_BANG: 
		compiler->emitByte(OP_NOT);
		break;
	case TOKEN_MINUS:
		compiler->emitByte(OP_NEGATE);
		break;
	default:return; // Unreachable
	}
}

void Parser::binary(bool)
{
	TokenType operatorType = previous.type;
	ParseRule* rule = getRule(operatorType);
	parsePrecedence((Precedence)(rule->precedence + 1));

	switch (operatorType) 
	{
	case TOKEN_BANG_EQUAL:    compiler->emitBytes(OP_EQUAL, OP_NOT); break;
	case TOKEN_EQUAL_EQUAL:   compiler->emitByte(OP_EQUAL); break;
	case TOKEN_GREATER:       compiler->emitByte(OP_GREATER); break;
	case TOKEN_GREATER_EQUAL: compiler->emitBytes(OP_LESS, OP_NOT); break;
	case TOKEN_LESS:          compiler->emitByte(OP_LESS); break;
	case TOKEN_LESS_EQUAL:    compiler->emitBytes(OP_GREATER, OP_NOT); break;
	case TOKEN_PLUS:          compiler->emitByte(OP_ADD); break;
	case TOKEN_MINUS:         compiler->emitByte(OP_SUBTRACT); break;
	case TOKEN_STAR:          compiler->emitByte(OP_MULTIPLY); break;
	case TOKEN_SLASH:         compiler->emitByte(OP_DIVIDE); break;
	default: return; // Unreachable.
	}
}

void Parser::call(bool)
{
	uint8_t argCount = argumentList();
	compiler->emitBytes(OP_CALL, argCount);
}

void Parser::literal(bool)
{
	switch (previous.type)
	{
	case TOKEN_FALSE: compiler->emitByte(OP_FALSE); break;
	case TOKEN_NIL: compiler->emitByte(OP_NIL); break;
	case TOKEN_TRUE: compiler->emitByte(OP_TRUE); break;
	default: return; // Unreachable.
	}
}

void Parser::and_(bool)
{
	// If value is false, jump to end directly. Else, try other operand.
	int endJump = compiler->emitJump(OP_JUMP_IF_FALSE);

	// Pop previous value, as only the last evaluated value is important
	compiler->emitByte(OP_POP);
	parsePrecedence(PREC_AND);

	compiler->patchJump(endJump);
}

void Parser::or_(bool)
{
	//TODO: Implement more instructions to make OR faster, or at least as fast as AND

	// If value is false, keep evaluating until a true value is found.
	int elseJump = compiler->emitJump(OP_JUMP_IF_FALSE);
	// If value was true, jump to the end directly
	int endJump = compiler->emitJump(OP_JUMP);

	compiler->patchJump(elseJump);
	// Pop previous value as only last evaluated value is important
	compiler->emitByte(OP_POP);

	parsePrecedence(PREC_OR);
	compiler->patchJump(endJump);
}

void Parser::variable(bool canAssign)
{
	namedVariable(previous, canAssign);
}

void Parser::namedVariable(Token name, bool canAssign)
{
	uint8_t getOp, setOp;
	int arg = resolveLocal(&compiler->locals, &name);
	if (arg != -1)
	{
		getOp = OP_GET_LOCAL;
		setOp = OP_SET_LOCAL;
	}
	else
	{
		arg = identifierConstant(&name);
		getOp = OP_GET_GLOBAL;
		setOp = OP_SET_GLOBAL;
	}

	if (canAssign && match(TOKEN_EQUAL))
	{
		expression();
		compiler->emitBytes(setOp, static_cast<uint8_t>(arg));
	}
	else
		compiler->emitBytes(getOp, static_cast<uint8_t>(arg));
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
	bool canAssign = precedence <= PREC_ASSIGNMENT;
	prefixRule(*this, canAssign);

	while (precedence <= getRule(current.type)->precedence)
	{
		advance();
		ParseFn infixRule = getRule(previous.type)->infix;
		infixRule(*this, canAssign);
	}

	if (canAssign && match(TOKEN_EQUAL))
		error(*this, "Invalid assignment target.");
}

uint8_t Parser::parseVariable(const char* errorMessage)
{
	consume(TOKEN_IDENTIFIER, errorMessage);

	declareVariable();
	// If we are in a local scope, return immediatly, as this is not a global variable and thus does not need the global constant name identifier
	if (compiler->locals.scopeDepth > 0)
		return 0;

	return identifierConstant(&previous);
}

uint8_t Parser::identifierConstant(Token* name)
{
	return compiler->makeConstant({vm, name->lexeme });
}

bool Parser::identifierEquals(const Token& a, const Token& b)
{
	// No need to compare length first as the == operator does it for us.
	return a.lexeme == b.lexeme;
}

int Parser::resolveLocal(Locals* locals, Token* name)
{
	for (int i = locals->localCount - 1; i >= 0; --i)
	{
		Local& local = locals->locals[i];
		if (identifierEquals(*name, local.name))
		{
			if (local.depth == -1)
				error(*this, "Can't read local variable in its own initializer.");
			return i;
		}
	}

	return -1;
}

void Parser::addLocal(const Token& name)
{
	if (compiler->locals.localCount == UINT8_COUNT)
	{
		error(*this, "Too many local variables in function.");
		return;
	}
	// Get next local and increase count
	Local* local = &compiler->locals.locals[compiler->locals.localCount++];
	
	local->name = name;
	local->depth = -1;
}

void Parser::defineVariable(uint8_t global)
{
	// If we are in a local scope, there is no need to create a global variable.
	// There also is no need to emit any byte code as a local variable lives on the stack.
	if (compiler->locals.scopeDepth > 0)
	{
		// Variable is usable after this point
		markInitialized();
		return;
	}
	compiler->emitBytes(OP_DEFINE_GLOBAL, global);
}

uint8_t Parser::argumentList()
{
	uint8_t argCount = 0;
	if (!check(TOKEN_RIGHT_PAREN))
	{
		do
		{
			expression();
			if (argCount == 255)
				error(*this, "Can't have more than 255 arguments.");
			++argCount;
		} while (match(TOKEN_COMMA));
	}
	consume(TOKEN_RIGHT_PAREN, "Expect ')' after arguments.");
	return argCount;
}

void Parser::markInitialized()
{
	// Don't try to mark a global variable as initialized in the local variables
	if (compiler->locals.scopeDepth == 0)
		return;
	compiler->locals.locals[compiler->locals.localCount - 1].depth = compiler->locals.scopeDepth;
}

void Parser::declareVariable()
{
	// Do not declare a global variable as a local variable in the compiler
	if (compiler->locals.scopeDepth == 0)
		return;

	const Token& name = previous;
	for (int i = compiler->locals.localCount - 1; i >= 0; --i)
	{
		const Local& local = compiler->locals.locals[i];
		// Exit loop if depth we are beyond current depth
		if (local.depth != -1 && local.depth < compiler->locals.scopeDepth)
			break;

		if (identifierEquals(name, local.name))
			error(*this, "Already a variable with this name in this scope.");
	}
	addLocal(name);
}

int Compiler::emitJump(uint8_t instruction)
{
	// Emit jump
	emitByte(instruction);
	// Emit placeholder jump destination
	emitByte(0xff);
	emitByte(0xff);

	// Return jump placeholder offset to change later using patchJump
	return static_cast<int>(currentChunk()->code.size() - 2);
}

void Compiler::patchJump(int offset)
{
	// -2 to adjust for the bytecode for the jump offset itself.
	int jump = static_cast<int>(currentChunk()->code.size() - offset - 2);

	// Check that jump is not too big, as we only have a int16 as operand
	if (jump > UINT16_MAX) {
		error(parser, "Too much code to jump over.");
	}

	// Cast jump as 2 uint8 values
	currentChunk()->code[offset] = (jump >> 8) & 0xff;
	currentChunk()->code[offset + 1] = jump & 0xff;
}

void Compiler::emitLoop(int loopStart)
{
	emitByte(OP_LOOP);

	int offset = static_cast<int>(currentChunk()->code.size() - loopStart + 2);
	if (offset > UINT16_MAX)
		error(parser, "Loop body too large.");

	emitByte((offset >> 8) & 0xff);
	emitByte(offset & 0xff);
}

bool Parser::match(TokenType type)
{
	if (!check(type))
		return false;
	advance();
	return true;
}

bool Parser::check(TokenType type)
{
	return current.type == type;
}

void Parser::InitRules()
{
	ParseRule tempRules[TOKEN_COUNT]{
 {&Parser::grouping, &Parser::call,   PREC_CALL},// [TOKEN_LEFT_PAREN]
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
 {&Parser::variable,     NULL,   PREC_NONE},// [TOKEN_IDENTIFIER]
 {&Parser::string,     NULL,   PREC_NONE},// [TOKEN_STRING]
 {&Parser::number,   NULL,   PREC_NONE},// [TOKEN_NUMBER]
 {NULL,     &Parser::and_,   PREC_AND},// [TOKEN_AND]
 {NULL,     NULL,   PREC_NONE},// [TOKEN_CLASS]
 {NULL,     NULL,   PREC_NONE},// [TOKEN_ELSE]
 {&Parser::literal,     NULL,   PREC_NONE},// [TOKEN_FALSE]
 {NULL,     NULL,   PREC_NONE},// [TOKEN_FOR]
 {NULL,     NULL,   PREC_NONE},// [TOKEN_FUN]
 {NULL,     NULL,   PREC_NONE},// [TOKEN_IF]
 {&Parser::literal,     NULL,   PREC_NONE},// [TOKEN_NIL]
 {NULL,     &Parser::or_,   PREC_OR},// [TOKEN_OR]
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
