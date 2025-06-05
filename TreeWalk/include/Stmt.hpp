#pragma once
#include "Token.h"
#include "Expr.hpp"
#include <memory>

class StmtVisitor
{
public:
	virtual Object visitExpressionStmt(class Expression& stmt) = 0;
	virtual Object visitPrintStmt(class Print& stmt) = 0;
	virtual ~StmtVisitor() = default; 
};

class Stmt
{
public:
virtual ~Stmt() = default; 
	virtual Object accept(StmtVisitor& visitor) = 0; 
}; 

class Expression : public Stmt
{
public:
	virtual ~Expression() override {}; 

	Expression(const std::shared_ptr<Expr>& expression) : expression{ expression }
	{}
	std::shared_ptr<Expr>	expression;

virtual Object accept(StmtVisitor& visitor) 
	{
		return visitor.visitExpressionStmt(*this);
	}
}; 

class Print : public Stmt
{
public:
	virtual ~Print() override {}; 

	Print(const std::shared_ptr<Expr>& expression) : expression{ expression }
	{}
	std::shared_ptr<Expr>	expression;

virtual Object accept(StmtVisitor& visitor) 
	{
		return visitor.visitPrintStmt(*this);
	}
}; 

