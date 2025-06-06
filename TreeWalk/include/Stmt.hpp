#pragma once
#include "Token.h"
#include "Object.h"
#include <list>
#include <memory>

class StmtVisitor
{
public:
	virtual Object visitBlockStmt(class Block& stmt) = 0;
	virtual Object visitExpressionStmt(class Expression& stmt) = 0;
	virtual Object visitPrintStmt(class Print& stmt) = 0;
	virtual Object visitVarStmt(class Var& stmt) = 0;
	virtual ~StmtVisitor() = default; 
};

class Stmt
{
public:
virtual ~Stmt() = default; 
	virtual Object accept(StmtVisitor& visitor) = 0; 
}; 

class Block : public Stmt
{
public:
	virtual ~Block() override {}; 

	Block(const std::list<class std::shared_ptr<class Stmt>>& statements) : statements{ statements }
	{}
	std::list<class std::shared_ptr<class Stmt>>	statements;

virtual Object accept(StmtVisitor& visitor) 
	{
		return visitor.visitBlockStmt(*this);
	}
}; 

class Expression : public Stmt
{
public:
	virtual ~Expression() override {}; 

	Expression(const std::shared_ptr<class Expr>& expression) : expression{ expression }
	{}
	std::shared_ptr<class Expr>	expression;

virtual Object accept(StmtVisitor& visitor) 
	{
		return visitor.visitExpressionStmt(*this);
	}
}; 

class Print : public Stmt
{
public:
	virtual ~Print() override {}; 

	Print(const std::shared_ptr<class Expr>& expression) : expression{ expression }
	{}
	std::shared_ptr<class Expr>	expression;

virtual Object accept(StmtVisitor& visitor) 
	{
		return visitor.visitPrintStmt(*this);
	}
}; 

class Var : public Stmt
{
public:
	virtual ~Var() override {}; 

	Var(const std::shared_ptr<class Token>& name, const std::shared_ptr<class Expr>& initializer) : name{ name }, initializer{ initializer }
	{}
	std::shared_ptr<class Token>	name;
	std::shared_ptr<class Expr>	initializer;

virtual Object accept(StmtVisitor& visitor) 
	{
		return visitor.visitVarStmt(*this);
	}
}; 

