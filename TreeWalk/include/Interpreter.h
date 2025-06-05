#pragma once

#include "Expr.hpp"
#include "Stmt.hpp"

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
	bool	isTruthy(Object& object);
	void	checkNumberOperand(Token op, Object& operand);
	void	checkNumberOperands(Token op, Object& left, Object& right);

	void	execute(Stmt& stmt);

	std::string	stringify(Object& object);
public:
	Object	evaluate(Expr& expr);

	Object	visitBinaryExpr(Binary& expr) override;
	Object	visitGroupingExpr(Grouping& expr) override;
	Object	visitLiteralExpr(Literal& expr) override;
	Object	visitUnaryExpr(Unary& expr) override;

	void	interpret(std::list<std::shared_ptr<Stmt>>& expr);

	Object visitExpressionStmt(Expression& stmt) override;
	Object visitPrintStmt(Print& stmt) override;
};

