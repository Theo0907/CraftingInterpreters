#pragma once
#include "Token.h"
#include <memory>

class ExprVisitor
{
public:
	virtual Object visitAssignExpr(class Assign& expr) = 0;
	virtual Object visitBinaryExpr(class Binary& expr) = 0;
	virtual Object visitGroupingExpr(class Grouping& expr) = 0;
	virtual Object visitLiteralExpr(class Literal& expr) = 0;
	virtual Object visitUnaryExpr(class Unary& expr) = 0;
	virtual Object visitVariableExpr(class Variable& expr) = 0;
	virtual ~ExprVisitor() = default; 
};

class Expr
{
public:
virtual ~Expr() = default; 
	virtual Object accept(ExprVisitor& visitor) = 0; 
}; 

class Assign : public Expr
{
public:
	virtual ~Assign() override {}; 

	Assign(const std::shared_ptr<Token>& name, const std::shared_ptr<Expr>& value) : name{ name }, value{ value }
	{}
	std::shared_ptr<Token>	name;
	std::shared_ptr<Expr>	value;

virtual Object accept(ExprVisitor& visitor) 
	{
		return visitor.visitAssignExpr(*this);
	}
}; 

class Binary : public Expr
{
public:
	virtual ~Binary() override {}; 

	Binary(const std::shared_ptr<Expr>& left, const std::shared_ptr<Token>& op, const std::shared_ptr<Expr>& right) : left{ left }, op{ op }, right{ right }
	{}
	std::shared_ptr<Expr>	left;
	std::shared_ptr<Token>	op;
	std::shared_ptr<Expr>	right;

virtual Object accept(ExprVisitor& visitor) 
	{
		return visitor.visitBinaryExpr(*this);
	}
}; 

class Grouping : public Expr
{
public:
	virtual ~Grouping() override {}; 

	Grouping(const std::shared_ptr<Expr>& expression) : expression{ expression }
	{}
	std::shared_ptr<Expr>	expression;

virtual Object accept(ExprVisitor& visitor) 
	{
		return visitor.visitGroupingExpr(*this);
	}
}; 

class Literal : public Expr
{
public:
	virtual ~Literal() override {}; 

	Literal(const std::shared_ptr<Object>& value) : value{ value }
	{}
	std::shared_ptr<Object>	value;

virtual Object accept(ExprVisitor& visitor) 
	{
		return visitor.visitLiteralExpr(*this);
	}
}; 

class Unary : public Expr
{
public:
	virtual ~Unary() override {}; 

	Unary(const std::shared_ptr<Token>& op, const std::shared_ptr<Expr>& right) : op{ op }, right{ right }
	{}
	std::shared_ptr<Token>	op;
	std::shared_ptr<Expr>	right;

virtual Object accept(ExprVisitor& visitor) 
	{
		return visitor.visitUnaryExpr(*this);
	}
}; 

class Variable : public Expr
{
public:
	virtual ~Variable() override {}; 

	Variable(const std::shared_ptr<Token>& name) : name{ name }
	{}
	std::shared_ptr<Token>	name;

virtual Object accept(ExprVisitor& visitor) 
	{
		return visitor.visitVariableExpr(*this);
	}
}; 

