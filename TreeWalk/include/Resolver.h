#pragma once

#include "expr.hpp"
#include "Stmt.hpp"
#include "Interpreter.h"

#include <stack>

class Resolver : public ExprVisitor, public StmtVisitor
{
protected:
	enum class FunctionType
	{
		NONE,
		FUNCTION,
		INITIALIZER,
		METHOD,
	};
	enum class ClassType
	{
		NONE,
		CLASS,
		SUBCLASS,
	};
	FunctionType	currentFunction =	FunctionType::NONE;
	ClassType		currentClass =		ClassType::NONE;
public:
	Interpreter* interpreter;
	std::stack<std::unordered_map<std::string, bool>> scopes;

	Resolver(Interpreter* interpreter) : interpreter{ interpreter }
	{ }

	Object visitAssignExpr(Assign& expr) override;
	Object visitBinaryExpr(Binary& expr) override;
	Object visitCallExpr(Call& expr) override;
	Object visitGroupingExpr(Grouping& expr) override;
	Object visitLiteralExpr(Literal& expr) override;
	Object visitLogicalExpr(Logical& expr) override;
	Object visitUnaryExpr(Unary& expr) override;
	Object visitVariableExpr(Variable& expr) override;
	Object visitGetExpr(Get& expr) override;
	Object visitSetExpr(Set& expr) override;
	Object visitThisExpr(This& expr) override;
	Object visitSuperExpr(Super& expr) override;

	Object visitBlockStmt(Block& stmt) override;
	Object visitExpressionStmt(Expression& stmt) override;
	Object visitFunctionStmt(Function& stmt) override;
	Object visitIfStmt(If& stmt) override;
	Object visitPrintStmt(Print& stmt) override;
	Object visitReturnStmt(Return& stmt) override;
	Object visitVarStmt(Var& stmt) override;
	Object visitWhileStmt(While& stmt) override;
	Object visitClassStmt(Class& stmt) override;

	void	resolve(std::list<std::shared_ptr<Stmt>> statements);
	void	resolve(const std::shared_ptr<Stmt>& stmt);
	void	resolve(const std::shared_ptr<Expr>& expr);
	void	resolveLocal(Expr& expr, Token name);
	void	resolveFunction(const Function& func, FunctionType type);
	void	beginScope();
	void	endScope();
	void	declare(Token name);
	void	define(Token name);
};

