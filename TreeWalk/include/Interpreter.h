#pragma once

#include "Expr.hpp"
#include "Stmt.hpp"
#include "Environment.h"

#include <stdexcept>
#include <list>

class RuntimeError : public std::runtime_error
{
public:
	RuntimeError() : std::runtime_error("A runtime error has occured.")
	{}
	RuntimeError(const Token& token, const std::string& message) : std::runtime_error(message + "\n[line" + std::to_string(token.line) + "]")
	{}
};

class Interpreter :
    public ExprVisitor, public StmtVisitor
{
protected:
	std::shared_ptr<Environment> environment = std::make_shared<Environment>();

	class ScopedEnvironment
	{
		Interpreter* interpreter;
		std::shared_ptr<Environment> prevEnvironment;
	public:
		ScopedEnvironment(class Interpreter* interpreter, const std::shared_ptr<Environment>& newEnvironment) : interpreter{ interpreter }
		{
			prevEnvironment = interpreter->environment;
			interpreter->environment = newEnvironment;
		}
		~ScopedEnvironment()
		{
			interpreter->environment = prevEnvironment;
		}
	};
	friend ScopedEnvironment;

	bool	isTruthy(const Object& object);
	void	checkNumberOperand(Token op, const Object& operand);
	void	checkNumberOperands(Token op, const Object& left, const Object& right);

	void	execute(Stmt& stmt);

	std::string	stringify(Object& object);
public:
	Object	evaluate(Expr& expr);

	Object	visitBinaryExpr(Binary& expr) override;
	Object	visitGroupingExpr(Grouping& expr) override;
	Object	visitLiteralExpr(Literal& expr) override;
	Object	visitUnaryExpr(Unary& expr) override;
	Object	visitVariableExpr(Variable& expr) override;
	Object	visitAssignExpr(Assign& expr) override;
	Object	visitLogicalExpr(Logical& expr) override;

	void	interpret(std::list<std::shared_ptr<Stmt>>& expr);

	Object	visitExpressionStmt(Expression& stmt) override;
	Object	visitPrintStmt(Print& stmt) override;
	Object	visitVarStmt(Var& stmt) override;
	Object	visitBlockStmt(Block& stmt) override;
	Object	visitIfStmt(If& stmt) override;

	void	executeBlock(const std::list<std::shared_ptr<class Stmt>>& statements, const std::shared_ptr<Environment>& environment);


	// Hérité via ExprVisitor

	// Hérité via StmtVisitor
};

