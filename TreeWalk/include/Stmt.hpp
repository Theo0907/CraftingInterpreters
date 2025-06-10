#pragma once
#include "Token.h"
#include "Object.h"
#include <list>
#include "Expr.hpp"
#include <memory>

class StmtVisitor
{
public:
	virtual Object visitBlockStmt(class Block& stmt) = 0;
	virtual Object visitFunctionStmt(class Function& stmt) = 0;
	virtual Object visitClassStmt(class Class& stmt) = 0;
	virtual Object visitExpressionStmt(class Expression& stmt) = 0;
	virtual Object visitIfStmt(class If& stmt) = 0;
	virtual Object visitPrintStmt(class Print& stmt) = 0;
	virtual Object visitReturnStmt(class Return& stmt) = 0;
	virtual Object visitVarStmt(class Var& stmt) = 0;
	virtual Object visitWhileStmt(class While& stmt) = 0;
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

class Function : public Stmt
{
public:
	virtual ~Function() override {}; 

	Function(const std::shared_ptr<class Token>& name, const std::list<class Token>& params, const std::list<class std::shared_ptr<class Stmt>>& body) : name{ name }, params{ params }, body{ body }
	{}
	std::shared_ptr<class Token>	name;
	std::list<class Token>	params;
	std::list<class std::shared_ptr<class Stmt>>	body;

virtual Object accept(StmtVisitor& visitor) 
	{
		return visitor.visitFunctionStmt(*this);
	}
}; 

class Class : public Stmt
{
public:
	virtual ~Class() override {}; 

	Class(const std::shared_ptr<class Token>& name, const std::list<class std::shared_ptr<class Function>>& methods) : name{ name }, methods{ methods }
	{}
	std::shared_ptr<class Token>	name;
	std::list<class std::shared_ptr<class Function>>	methods;

virtual Object accept(StmtVisitor& visitor) 
	{
		return visitor.visitClassStmt(*this);
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

class If : public Stmt
{
public:
	virtual ~If() override {}; 

	If(const std::shared_ptr<class Expr>& condition, const std::shared_ptr<class Stmt>& thenBranch, const std::shared_ptr<class Stmt>& elseBranch) : condition{ condition }, thenBranch{ thenBranch }, elseBranch{ elseBranch }
	{}
	std::shared_ptr<class Expr>	condition;
	std::shared_ptr<class Stmt>	thenBranch;
	std::shared_ptr<class Stmt>	elseBranch;

virtual Object accept(StmtVisitor& visitor) 
	{
		return visitor.visitIfStmt(*this);
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

class Return : public Stmt
{
public:
	virtual ~Return() override {}; 

	Return(const std::shared_ptr<class Token>& keyword, const std::shared_ptr<class Expr>& value) : keyword{ keyword }, value{ value }
	{}
	std::shared_ptr<class Token>	keyword;
	std::shared_ptr<class Expr>	value;

virtual Object accept(StmtVisitor& visitor) 
	{
		return visitor.visitReturnStmt(*this);
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

class While : public Stmt
{
public:
	virtual ~While() override {}; 

	While(const std::shared_ptr<class Expr>& condition, const std::shared_ptr<class Stmt>& body) : condition{ condition }, body{ body }
	{}
	std::shared_ptr<class Expr>	condition;
	std::shared_ptr<class Stmt>	body;

virtual Object accept(StmtVisitor& visitor) 
	{
		return visitor.visitWhileStmt(*this);
	}
}; 

