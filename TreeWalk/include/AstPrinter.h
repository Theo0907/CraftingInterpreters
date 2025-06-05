#pragma once

#include "Expr.hpp"

#include <iostream>

template<class T>
concept ExprDerived = std::is_base_of<Expr, T>::value;

class AstPrinter :
    public Visitor
{
protected:
	template <ExprDerived... Types>
	void parenthesize_internal(std::string& result, std::string name, Expr& expr, Types&... args)
	{
		result.append(" ");
		auto a = expr.accept(*this);
		std::string s = a.GetString();
		result.append(s);

		if constexpr (sizeof...(args) > 0)
			parenthesize_internal(result, name, args...);
	}

	template <ExprDerived... Types>
	Object parenthesize(std::string name, Expr& expr, Types&... args)
	{
		std::string result;
		result.append("(").append(name);
		
		parenthesize_internal(result, name, expr, args...);

		result.append(")");
		
		return result;
	}

public:
	virtual ~AstPrinter() override = default;

	Object	print(Expr& expr);

	Object	visitBinaryExpr(Binary& expr) override;
	Object	visitGroupingExpr(Grouping& expr) override;
	Object	visitLiteralExpr(Literal& expr) override;
	Object	visitUnaryExpr(Unary& expr) override;
};

