#pragma once

#include "Expr.hpp"
#include "Stmt.hpp"
#include "Environment.h"
#include "LoxCallable.h"

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
public:
	std::shared_ptr<Environment> globals = std::make_shared<Environment>();
protected:
	std::shared_ptr<Environment> environment = globals;

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

	class ClockNativeFunc : public LoxCallable
	{
	public:
		virtual ~ClockNativeFunc() override;
		virtual Object call(const Interpreter& interpreter, const std::list<Object>& arguments) override;
		virtual int arity() const override;
	};

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
	Object	visitCallExpr(Call& expr) override;

	void	interpret(std::list<std::shared_ptr<Stmt>>& expr);

	Object	visitExpressionStmt(Expression& stmt) override;
	Object	visitPrintStmt(Print& stmt) override;
	Object	visitVarStmt(Var& stmt) override;
	Object	visitBlockStmt(Block& stmt) override;
	Object	visitIfStmt(If& stmt) override;
	Object	visitWhileStmt(While& stmt) override;

	void	executeBlock(const std::list<std::shared_ptr<class Stmt>>& statements, const std::shared_ptr<Environment>& environment);

	Interpreter()
	{
		globals->define("clock", new ClockNativeFunc());
	}
	~Interpreter()
	{
		// delete allocated pointer (need to make a better way to do this, such as a manager for native funcs)
		const Object& o = globals->get(Token(Token::TokenType::FUN, "clock", {}, 0));
		if (o.IsFunction())
			delete o.GetFunction();
	}
};

