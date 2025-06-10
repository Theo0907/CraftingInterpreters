#include "Resolver.h"

#include "TreeWalk.h"

Object Resolver::visitAssignExpr(Assign& expr)
{
	resolve(expr.value);
	resolveLocal(expr, *expr.name);
	return {};
}

Object Resolver::visitBinaryExpr(Binary& expr)
{
	resolve(expr.left);
	resolve(expr.right);
	return {};
}

Object Resolver::visitCallExpr(Call& expr)
{
	resolve(expr.callee);

	for (auto arg : expr.arguments)
		resolve(arg);

	return {};
}

Object Resolver::visitGroupingExpr(Grouping& expr)
{
	resolve(expr.expression);
	return {};
}

Object Resolver::visitLiteralExpr(Literal&)
{
	return {};
}

Object Resolver::visitLogicalExpr(Logical& expr)
{
	resolve(expr.left);
	resolve(expr.right);

	return {};
}

Object Resolver::visitUnaryExpr(Unary& expr)
{
	resolve(expr.right);
	return {};
}

Object Resolver::visitVariableExpr(Variable& expr)
{
	if (!scopes.empty())
	{
		auto it = scopes.top().find(expr.name->lexeme);
		if (it != scopes.top().end() && !it->second)
			TreeWalk::Error(*expr.name, "Can't read local variable in its own initializer");
	}

	resolveLocal(expr, *expr.name);
	return {};
}

Object Resolver::visitBlockStmt(Block& stmt)
{
	beginScope();
	resolve(stmt.statements);
	endScope();
	return {};
}

Object Resolver::visitExpressionStmt(Expression& stmt)
{
	resolve(stmt.expression);
	return {};
}

Object Resolver::visitFunctionStmt(Function& stmt)
{
	declare(*stmt.name);
	define(*stmt.name);

	resolveFunction(stmt, FunctionType::FUNCTION);
	return {};
}

Object Resolver::visitIfStmt(If& stmt)
{
	resolve(stmt.condition);
	resolve(stmt.thenBranch);
	if (stmt.elseBranch)
		resolve(stmt.elseBranch);
	return {};
}

Object Resolver::visitPrintStmt(Print& stmt)
{
	resolve(stmt.expression);
	return {};
}

Object Resolver::visitReturnStmt(Return& stmt)
{
	if (currentFunction == FunctionType::NONE)
		TreeWalk::Error(*stmt.keyword, "Can't return from top-level code.");
	if (stmt.value)
	{
		if (currentFunction == FunctionType::INITIALIZER)
			TreeWalk::Error(*stmt.keyword, "Can't return a value from an initializer.");
		resolve(stmt.value);
	}
	return {};
}

Object Resolver::visitVarStmt(Var& stmt)
{
	declare(*stmt.name);
	if (stmt.initializer)
		resolve(stmt.initializer);
	define(*stmt.name);
	return {};
}

Object Resolver::visitWhileStmt(While& stmt)
{
	resolve(stmt.condition);
	resolve(stmt.body);
	return {};
}

void Resolver::resolve(std::list<std::shared_ptr<Stmt>> statements)
{
	for (const std::shared_ptr<Stmt>& statement : statements)
		resolve(statement);
}

void Resolver::resolve(const std::shared_ptr<Stmt>& stmt)
{
	stmt->accept(*this);
}

void Resolver::resolve(const std::shared_ptr<Expr>& expr)
{
	expr->accept(*this);
}

void Resolver::resolveLocal(Expr& expr, Token name)
{
	auto scopesCopy = scopes;
	while (!scopesCopy.empty())
	{
		std::unordered_map<std::string, bool>	scope = scopesCopy.top();

		if (scope.contains(name.lexeme))
		{
			interpreter->resolve(expr, (int)scopes.size() - (int)scopesCopy.size());
			return;
		}
		scopesCopy.pop();
	}
}

void Resolver::resolveFunction(const Function& func, FunctionType type)
{
	FunctionType enclosing = currentFunction;
	currentFunction = type;
	beginScope();
	for (const Token& param : func.params)
	{
		declare(param);
		define(param);
	}
	resolve(func.body);
	endScope();
	currentFunction = enclosing;
}

void Resolver::beginScope()
{
	scopes.push({});
}

void Resolver::endScope()
{
	scopes.pop();
}

void Resolver::declare(Token name)
{
	if (scopes.empty())
		return;

	std::unordered_map<std::string, bool>& scope = scopes.top();
	if (scope.contains(name.lexeme))
		TreeWalk::Error(name, "Already a variable with this name in this scope.");
	scope.emplace(name.lexeme, false);
}

void Resolver::define(Token name)
{
	if (scopes.empty())
		return;
	scopes.top()[name.lexeme] = true;
}

Object Resolver::visitGetExpr(Get& expr)
{
	resolve(expr.object);
	return {};
}

Object Resolver::visitSetExpr(Set& expr)
{
	resolve(expr.value);
	resolve(expr.object);
	return {};
}

Object Resolver::visitThisExpr(This& expr)
{
	if (currentClass == ClassType::NONE)
	{
		TreeWalk::Error(*expr.keyword, "Can't use 'this' outside of a class.");
		return {};
	}
	resolveLocal(expr, *expr.keyword);
	return {};
}

Object Resolver::visitClassStmt(Class& stmt)
{
	ClassType enclosingClass = currentClass;
	currentClass = ClassType::CLASS;
	declare(*stmt.name);
	define(*stmt.name);

	beginScope();
	scopes.top()["this"] = true;

	for (const std::shared_ptr<Function>& method : stmt.methods)
	{
		// This could have been a ternary but is clearer like this (I hope)
		FunctionType decl = FunctionType::METHOD;
		if (method->name->lexeme == "init")
			decl = FunctionType::INITIALIZER;
		resolveFunction(*method, decl);
	}

	endScope();
	currentClass = enclosingClass;

	return {};
}
