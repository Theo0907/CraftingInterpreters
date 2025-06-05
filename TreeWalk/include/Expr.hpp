#pragma once
#include "Token.h"
#include <memory>

class Visitor
{
public:
	virtual Object visitBinaryExpr(class Binary& expr) = 0;
	virtual Object visitGroupingExpr(class Grouping& expr) = 0;
	virtual Object visitLiteralExpr(class Literal& expr) = 0;
	virtual Object visitUnaryExpr(class Unary& expr) = 0;
	virtual ~Visitor() = default;
};

class Expr
{
public:
virtual ~Expr() = default; 
	virtual Object accept(Visitor& visitor) = 0;
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

virtual Object accept(Visitor& visitor) 
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

virtual Object accept(Visitor& visitor) 
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

virtual Object accept(Visitor& visitor) 
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

virtual Object accept(Visitor& visitor) 
	{
		return visitor.visitUnaryExpr(*this);
	}
}; 

