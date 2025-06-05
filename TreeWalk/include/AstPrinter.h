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
		std::string s = std::get<std::string>(a);
		result.append(s);

		if constexpr (sizeof...(args) > 0)
			parenthesize_internal(result, name, args...);
	}

	template <ExprDerived... Types>
	std::variant<std::string, double, std::monostate> parenthesize(std::string name, Expr& expr, Types&... args)
	{
		std::string result;
		result.append("(").append(name);
		
		parenthesize_internal(result, name, expr, args...);

		result.append(")");
		
		return result;
	}

public:
	virtual ~AstPrinter() override = default;

	std::variant<std::string, double, std::monostate> print(Expr& expr);

	std::variant<std::string, double, std::monostate> visitBinaryExpr(Binary& expr) override;
	std::variant<std::string, double, std::monostate> visitGroupingExpr(Grouping& expr) override;
	std::variant<std::string, double, std::monostate> visitLiteralExpr(Literal& expr) override;
	std::variant<std::string, double, std::monostate> visitUnaryExpr(Unary& expr) override;
};

