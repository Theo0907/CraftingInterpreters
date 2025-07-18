#pragma once
#include "Token.h"
#include "Object.h"
#include <list>
#include <memory>

class ExprVisitor
{
public:
	virtual Object visitAssignExpr(class Assign& expr) = 0;
	virtual Object visitBinaryExpr(class Binary& expr) = 0;
	virtual Object visitCallExpr(class Call& expr) = 0;
	virtual Object visitGetExpr(class Get& expr) = 0;
	virtual Object visitGroupingExpr(class Grouping& expr) = 0;
	virtual Object visitLiteralExpr(class Literal& expr) = 0;
	virtual Object visitLogicalExpr(class Logical& expr) = 0;
	virtual Object visitSetExpr(class Set& expr) = 0;
	virtual Object visitSuperExpr(class Super& expr) = 0;
	virtual Object visitThisExpr(class This& expr) = 0;
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

	Assign(const std::shared_ptr<class Token>& name, const std::shared_ptr<class Expr>& value) : name{ name }, value{ value }
	{}
	std::shared_ptr<class Token>	name;
	std::shared_ptr<class Expr>	value;

virtual Object accept(ExprVisitor& visitor) 
	{
		return visitor.visitAssignExpr(*this);
	}
}; 

class Binary : public Expr
{
public:
	virtual ~Binary() override {}; 

	Binary(const std::shared_ptr<class Expr>& left, const std::shared_ptr<class Token>& op, const std::shared_ptr<class Expr>& right) : left{ left }, op{ op }, right{ right }
	{}
	std::shared_ptr<class Expr>	left;
	std::shared_ptr<class Token>	op;
	std::shared_ptr<class Expr>	right;

virtual Object accept(ExprVisitor& visitor) 
	{
		return visitor.visitBinaryExpr(*this);
	}
}; 

class Call : public Expr
{
public:
	virtual ~Call() override {}; 

	Call(const std::shared_ptr<class Expr>& callee, const std::shared_ptr<class Token>& paren, const std::list<class std::shared_ptr<class Expr>>& arguments) : callee{ callee }, paren{ paren }, arguments{ arguments }
	{}
	std::shared_ptr<class Expr>	callee;
	std::shared_ptr<class Token>	paren;
	std::list<class std::shared_ptr<class Expr>>	arguments;

virtual Object accept(ExprVisitor& visitor) 
	{
		return visitor.visitCallExpr(*this);
	}
}; 

class Get : public Expr
{
public:
	virtual ~Get() override {}; 

	Get(const std::shared_ptr<class Expr>& object, const std::shared_ptr<class Token>& name) : object{ object }, name{ name }
	{}
	std::shared_ptr<class Expr>	object;
	std::shared_ptr<class Token>	name;

virtual Object accept(ExprVisitor& visitor) 
	{
		return visitor.visitGetExpr(*this);
	}
}; 

class Grouping : public Expr
{
public:
	virtual ~Grouping() override {}; 

	Grouping(const std::shared_ptr<class Expr>& expression) : expression{ expression }
	{}
	std::shared_ptr<class Expr>	expression;

virtual Object accept(ExprVisitor& visitor) 
	{
		return visitor.visitGroupingExpr(*this);
	}
}; 

class Literal : public Expr
{
public:
	virtual ~Literal() override {}; 

	Literal(const std::shared_ptr<class Object>& value) : value{ value }
	{}
	std::shared_ptr<class Object>	value;

virtual Object accept(ExprVisitor& visitor) 
	{
		return visitor.visitLiteralExpr(*this);
	}
}; 

class Logical : public Expr
{
public:
	virtual ~Logical() override {}; 

	Logical(const std::shared_ptr<class Expr>& left, const std::shared_ptr<class Token>& op, const std::shared_ptr<class Expr>& right) : left{ left }, op{ op }, right{ right }
	{}
	std::shared_ptr<class Expr>	left;
	std::shared_ptr<class Token>	op;
	std::shared_ptr<class Expr>	right;

virtual Object accept(ExprVisitor& visitor) 
	{
		return visitor.visitLogicalExpr(*this);
	}
}; 

class Set : public Expr
{
public:
	virtual ~Set() override {}; 

	Set(const std::shared_ptr<class Expr>& object, const std::shared_ptr<class Token>& name, const std::shared_ptr<class Expr>& value) : object{ object }, name{ name }, value{ value }
	{}
	std::shared_ptr<class Expr>	object;
	std::shared_ptr<class Token>	name;
	std::shared_ptr<class Expr>	value;

virtual Object accept(ExprVisitor& visitor) 
	{
		return visitor.visitSetExpr(*this);
	}
}; 

class Super : public Expr
{
public:
	virtual ~Super() override {}; 

	Super(const std::shared_ptr<class Token>& keyword, const std::shared_ptr<class Token>& method) : keyword{ keyword }, method{ method }
	{}
	std::shared_ptr<class Token>	keyword;
	std::shared_ptr<class Token>	method;

virtual Object accept(ExprVisitor& visitor) 
	{
		return visitor.visitSuperExpr(*this);
	}
}; 

class This : public Expr
{
public:
	virtual ~This() override {}; 

	This(const std::shared_ptr<class Token>& keyword) : keyword{ keyword }
	{}
	std::shared_ptr<class Token>	keyword;

virtual Object accept(ExprVisitor& visitor) 
	{
		return visitor.visitThisExpr(*this);
	}
}; 

class Unary : public Expr
{
public:
	virtual ~Unary() override {}; 

	Unary(const std::shared_ptr<class Token>& op, const std::shared_ptr<class Expr>& right) : op{ op }, right{ right }
	{}
	std::shared_ptr<class Token>	op;
	std::shared_ptr<class Expr>	right;

virtual Object accept(ExprVisitor& visitor) 
	{
		return visitor.visitUnaryExpr(*this);
	}
}; 

class Variable : public Expr
{
public:
	virtual ~Variable() override {}; 

	Variable(const std::shared_ptr<class Token>& name) : name{ name }
	{}
	std::shared_ptr<class Token>	name;

virtual Object accept(ExprVisitor& visitor) 
	{
		return visitor.visitVariableExpr(*this);
	}
}; 

