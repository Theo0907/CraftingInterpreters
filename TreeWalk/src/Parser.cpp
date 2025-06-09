#include "Parser.h"

#include "TreeWalk.h"

std::shared_ptr<Expr> Parser::expression()
{
	return assignment();
}

std::shared_ptr<Expr> Parser::assignment()
{
	std::shared_ptr<Expr> expr = orExpr();

	if (match({ Token::TokenType::EQUAL }))
	{
		Token equals = previous();
		std::shared_ptr<Expr> value = assignment();

		std::shared_ptr<Variable> asVariable = dynamic_pointer_cast<Variable>(expr);
		if (asVariable)
		{
			Token name = *asVariable->name;
			return std::make_shared<Assign>(std::make_shared<Token>(name), value);
		}

		// Don't throw as we don't need to synchronize
		error(equals, "Invalid assignment target.");
	}

	return expr;
}

std::shared_ptr<Expr> Parser::orExpr()
{
	std::shared_ptr<Expr> expr = andExpr();

	while (match({ Token::TokenType::OR }))
	{
		Token op = previous();
		std::shared_ptr<Expr> right = andExpr();
		expr = std::make_shared<Logical>(expr, std::make_shared<Token>(op), right);
	}

	return expr;
}

std::shared_ptr<Expr> Parser::andExpr()
{
	std::shared_ptr<Expr> expr = equality();

	while (match({ Token::TokenType::AND }))
	{
		Token op = previous();
		std::shared_ptr<Expr> right = equality();
		expr = std::make_shared<Logical>(expr, std::make_shared<Token>(op), right);
	}

	return expr;
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
	
	return call();
}

std::shared_ptr<Expr> Parser::call()
{
	std::shared_ptr<Expr> expr = primary();

	while (true)
	{
		if (match({ Token::TokenType::LEFT_PAREN }))
			expr = finishCall(expr);
		else
			break;
	}
	return expr;
}

std::shared_ptr<Expr> Parser::finishCall(std::shared_ptr<Expr> callee)
{
	std::list<std::shared_ptr<Expr>> args;

	if (!check(Token::TokenType::RIGHT_PAREN))
	{
		do
		{
			if (args.size() >= 255)
				error(peek(), "Can't have more than 255 arguments.");
			args.push_back(expression());
		}
		while (match({ Token::TokenType::COMMA }));
	}

	Token paren = consume(Token::TokenType::RIGHT_PAREN, "Expect ')' after arguments.");

	return std::make_shared<Call>(callee, std::make_shared<Token>(paren), args);
}

std::shared_ptr<Expr> Parser::primary()
{
	if (match({ Token::TokenType::FALSE })) return std::make_shared<Literal>(std::make_shared<Object>(false));
	if (match({ Token::TokenType::TRUE })) return std::make_shared<Literal>(std::make_shared<Object>(true));
	if (match({ Token::TokenType::NIL })) return std::make_shared<Literal>(std::make_shared<Object>());

	if (match({ Token::TokenType::NUMBER, Token::TokenType::STRING }))
		return std::make_shared<Literal>(std::make_shared<Object>(previous().literal));

	if (match({ Token::TokenType::IDENTIFIER }))
		return std::make_shared<Variable>(std::make_shared<Token>(previous()));

	if (match({ Token::TokenType::LEFT_PAREN }))
	{
		std::shared_ptr<Expr> expr = expression();
		consume(Token::TokenType::RIGHT_PAREN, "Expect ')' after expression.");
		return std::make_shared<Grouping>(expr);
	}

	throw error(peek(), "Expect expression.");
}

std::shared_ptr<Stmt> Parser::statement()
{
	if (match({ Token::TokenType::FOR }))
		return forStatement();
	if (match({ Token::TokenType::IF }))
		return ifStatement();
	if (match({ Token::TokenType::PRINT }))
		return printStatement();
	if (match({ Token::TokenType::WHILE }))
		return whileStatement();
	if (match({ Token::TokenType::LEFT_BRACE }))
		return std::make_shared<Block>(block());
	return expressionStatement();
}

std::shared_ptr<Stmt> Parser::printStatement()
{
	std::shared_ptr<Expr> value = expression();
	consume(Token::TokenType::SEMICOLON, "Expect ';' after value.");
	return std::make_shared<Print>(value);
}

std::shared_ptr<Stmt> Parser::forStatement()
{
	consume(Token::TokenType::LEFT_PAREN, "Expect '(' after 'for'.");

	// for (initializer;condition;increment)

	// Populating initializer
	std::shared_ptr<Stmt> init;
	if (match({ Token::TokenType::SEMICOLON }))
		init = nullptr;
	else if (match({ Token::TokenType::VAR }))
		init = varDeclaration();
	else
		init = expressionStatement();

	// Populating condition
	std::shared_ptr<Expr> cond = nullptr;
	if (!check(Token::TokenType::SEMICOLON))
		cond = expression();
	consume(Token::TokenType::SEMICOLON, "Expect ';' after loop condition.");

	// Populating increment
	std::shared_ptr<Expr> incr = nullptr;
	if (!check(Token::TokenType::RIGHT_PAREN))
		incr = expression();
	consume(Token::TokenType::RIGHT_PAREN, "Expect ')' after for clauses.");

	std::shared_ptr<Stmt> body = statement();

	// Adding increment to body if needed
	if (incr)
	{
		body = std::make_shared<Block>(std::list<std::shared_ptr<Stmt>>({
			body,
			(std::shared_ptr<Stmt>)std::make_shared<Expression>(incr)
			}));
	}

	// Adding condition to body and forcing it if empty
	if (!cond)
		cond = std::make_shared<Literal>(std::make_shared<Object>(true));
	body = std::make_shared<While>(cond, body);

	// Adding initializer to body if needed
	if (init)
	{
		body = std::make_shared<Block>(std::list<std::shared_ptr<Stmt>>({
			init,
			body
			}));
	}

	return body;
}

std::shared_ptr<Stmt> Parser::whileStatement()
{
	// TODO: Implement break statement
	consume(Token::TokenType::LEFT_PAREN, "Expect '(' after 'while'.");
	std::shared_ptr<Expr>	cond = expression();
	consume(Token::TokenType::RIGHT_PAREN, "Expect ')' after 'while' condition.");
	std::shared_ptr<Stmt>	body = statement();

	return std::make_shared<While>(cond, body);
}

std::shared_ptr<Stmt> Parser::ifStatement()
{
	consume(Token::TokenType::LEFT_PAREN, "Expect '(' after 'if'.");
	std::shared_ptr<Expr>	cond = expression();
	consume(Token::TokenType::RIGHT_PAREN, "Expect ')' after 'if' condition.");

	std::shared_ptr<Stmt> thenBranch = statement();
	std::shared_ptr<Stmt> elseBranch = nullptr;
	if (match({ Token::TokenType::ELSE }))
		elseBranch = statement();

	return std::make_shared<If>(cond, thenBranch, elseBranch);
}

std::shared_ptr<Stmt> Parser::expressionStatement()
{
	std::shared_ptr<Expr> expr = expression();
	consume(Token::TokenType::SEMICOLON, "Expect ';' after value.");
	return std::make_shared<Expression>(expr);
}

std::shared_ptr<Stmt> Parser::declaration()
{
	try
	{
		if (match({ Token::TokenType::VAR }))
			return varDeclaration();

		return statement();
	}
	catch (ParseError error)
	{
		synchronize();
		return nullptr;
	}
}

std::shared_ptr<Stmt> Parser::varDeclaration()
{
	Token name = consume(Token::TokenType::IDENTIFIER, "Expect variable name");

	std::shared_ptr<Expr> initializer = nullptr;
	if (match({ Token::TokenType::EQUAL }))
		initializer = expression();

	consume(Token::TokenType::SEMICOLON, "Expect ';' after variable declaration");
	return std::make_shared<Var>(std::make_shared<Token>(name), initializer);
}

std::list<std::shared_ptr<Stmt>> Parser::block()
{
	std::list<std::shared_ptr<Stmt>> statements;

	while (!check({ Token::TokenType::RIGHT_BRACE }) && !isAtEnd())
		statements.push_back(declaration());

	consume(Token::TokenType::RIGHT_BRACE, "Expect '}' after block.");
	return statements;
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

std::list<std::shared_ptr<Stmt>> Parser::parse()
{
	std::list<std::shared_ptr<Stmt>> statements;
	while (!isAtEnd())
		statements.push_back(declaration());
	return statements;
}
