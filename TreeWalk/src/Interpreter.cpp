#include "Interpreter.h"

#include "TreeWalk.h"
#include "LoxCallable.h"
#include "LoxFunction.h"

#include <format>
#include <chrono>

bool Interpreter::isTruthy(const Object& object)
{
	if (object.IsNull())
		return false;
	if (object.IsBool())
		return object.GetBool();

	// TODO: Maybe check something about number or empty string?
	return true;
}

void Interpreter::checkNumberOperand(Token op, const Object& operand)
{
	if (operand.IsNumber())
		return;
	throw RuntimeError(op, "Operand must be a number");
}

void Interpreter::checkNumberOperands(Token op, const Object& left, const Object& right)
{
	if (left.IsNumber() && right.IsNumber())
		return;

	throw RuntimeError(op, "Operands must be numbers.");
}

void Interpreter::execute(Stmt& stmt)
{
	stmt.accept(*this);
}

std::string Interpreter::stringify(Object& object)
{
	return object.GetString();
}

Object Interpreter::evaluate(Expr& expr)
{
	return expr.accept(*this);
}

Object Interpreter::visitBinaryExpr(Binary& expr)
{
	Object left{ evaluate(*expr.left) };
	Object right{ evaluate(*expr.right) };

	switch (expr.op->type)
	{
	case Token::TokenType::MINUS:
		checkNumberOperands(*expr.op, left, right);
		return left.GetNumber() - right.GetNumber();
	case Token::TokenType::SLASH:
		checkNumberOperands(*expr.op, left, right);
		return left.GetNumber() / right.GetNumber();
	case Token::TokenType::STAR:
		checkNumberOperands(*expr.op, left, right);
		return left.GetNumber() * right.GetNumber();
	case Token::TokenType::PLUS:
		if (left.IsNumber() && right.IsNumber())
			return left.GetNumber() + right.GetNumber();
		if (left.IsString() && right.IsString())
			return left.GetString() + right.GetString();

		throw RuntimeError(*expr.op, "Operands must be two numbers or two strings.");
	case Token::TokenType::GREATER:
		checkNumberOperands(*expr.op, left, right);
		return left.GetNumber() > right.GetNumber();
	case Token::TokenType::GREATER_EQUAL:
		checkNumberOperands(*expr.op, left, right);
		return left.GetNumber() >= right.GetNumber();
	case Token::TokenType::LESS:
		checkNumberOperands(*expr.op, left, right);
		return left.GetNumber() < right.GetNumber();
	case Token::TokenType::LESS_EQUAL:
		checkNumberOperands(*expr.op, left, right);
		return left.GetNumber() <= right.GetNumber();
	case Token::TokenType::BANG_EQUAL:
		return !(left == right);
	case Token::TokenType::EQUAL_EQUAL:
		return left == right;
	}

	return {};
}

Object Interpreter::visitGroupingExpr(Grouping& expr)
{
	return evaluate(*expr.expression);
}

Object Interpreter::visitLiteralExpr(Literal& expr)
{
	return *expr.value;
}

Object Interpreter::visitUnaryExpr(Unary& expr)
{
	Object right = evaluate(*expr.right);

	switch (expr.op->type)
	{
	case Token::TokenType::MINUS:
		checkNumberOperand(*expr.op, right);
		return -right.GetNumber();
	case Token::TokenType::BANG:
		return !isTruthy(right);
	}

	return {};
}

void Interpreter::interpret(std::list<std::shared_ptr<Stmt>>& statements)
{
	try
	{
		for (auto& it : statements)
			execute(*it);
	}
	catch (RuntimeError error)
	{
		TreeWalk::RuntimeError(error.what());
	}
}

Object Interpreter::visitExpressionStmt(Expression& stmt)
{
	evaluate(*stmt.expression);
	return {};
}

Object Interpreter::visitPrintStmt(Print& stmt)
{
	Object value = evaluate(*stmt.expression);
	std::cout << stringify(value) << std::endl;
	return {};
}

Object Interpreter::visitVariableExpr(Variable& expr)
{
	return lookUpVariable(*expr.name, expr);
}

Object Interpreter::visitVarStmt(Var& stmt)
{
	Object value;
	if (stmt.initializer != nullptr)
		value = evaluate(*stmt.initializer);

	environment->define(stmt.name->lexeme, value);
	return value;
}

Object Interpreter::visitBlockStmt(Block& stmt)
{
	executeBlock(stmt.statements, std::make_shared<Environment>(environment));
	return {};
}

void Interpreter::executeBlock(const std::list<std::shared_ptr<class Stmt>>& statements, const std::shared_ptr<Environment>& newEnvironment)
{
	ScopedEnvironment scoped(this, newEnvironment);

	for (const std::shared_ptr<Stmt> statement : statements)
		execute(*statement);
}

Object Interpreter::lookUpVariable(Token name, Expr& expr)
{
	auto it = locals.find(&expr);
	if (it != locals.end())
	{
		int dist = it->second;
		return environment->getAt(dist, name.lexeme);
	}
	else
		return globals->get(name);
}

void Interpreter::resolve(Expr& expr, int depth)
{
	locals.emplace(&expr, depth);
}

Object Interpreter::visitReturnStmt(Return& stmt)
{
	Object value;
	if (stmt.value)
		value = evaluate(*stmt.value);

	throw ReturnException(value);
}

Object Interpreter::visitFunctionStmt(Function& stmt)
{
	std::shared_ptr<LoxFunction> fun = std::make_shared<LoxFunction>(stmt, environment);
	environment->define(stmt.name->lexeme, { fun });
	return {};
}

Object Interpreter::visitCallExpr(Call& expr)
{
	Object callee = evaluate(*expr.callee);

	std::list<Object> args;
	for (const std::shared_ptr<Expr>& arg : expr.arguments)
		args.push_back(evaluate(*arg));

	if (!callee.IsFunction())
		throw RuntimeError(*expr.paren, "Can only call functions and classes.");

	std::shared_ptr<class LoxCallable> function = callee.GetFunction();

	if (args.size() != function->arity())
		throw RuntimeError(*expr.paren, std::format("Expected {} arguments but got {}.", function->arity(), args.size()));

	return function->call(*this, args);
}

Object Interpreter::visitWhileStmt(While& stmt)
{
	while (isTruthy(evaluate(*stmt.condition)))
		execute(*stmt.body);

	return {};
}

Object Interpreter::visitLogicalExpr(Logical& expr)
{
	Object left = evaluate(*expr.left);

	if (expr.op->type == Token::TokenType::OR)
	{
		if (isTruthy(left))
			return left;
	}
	else
	{
		if (!isTruthy(left))
			return left;
	}
	return evaluate(*expr.right);
}

Object Interpreter::visitIfStmt(If& stmt)
{
	if (isTruthy(evaluate(*stmt.condition)))
		execute(*stmt.thenBranch);
	else if (stmt.elseBranch)
		execute(*stmt.elseBranch);

	return {};
}

Object Interpreter::visitAssignExpr(Assign& expr)
{
	Object value = evaluate(*expr.value);

	auto it = locals.find(&expr);
	if (it != locals.end())
	{
		int dist = it->second;
		environment->assignAt(dist, *expr.name, value);
	}
	else
		globals->assign(*expr.name, value);
	return value;
}

Interpreter::ClockNativeFunc::~ClockNativeFunc()
{
}

Object Interpreter::ClockNativeFunc::call(Interpreter&, const std::list<Object>&)
{
	std::chrono::time_point<std::chrono::high_resolution_clock, std::chrono::nanoseconds> now = std::chrono::high_resolution_clock::now();
	return (double)now.time_since_epoch().count() / 1e9; // Return nanoseconds duration to seconds
}

int Interpreter::ClockNativeFunc::arity() const
{
	return 0;
}

std::string Interpreter::ClockNativeFunc::ToString() const
{
	return "<clock native fn>";
}
