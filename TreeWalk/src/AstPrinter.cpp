#include "AstPrinter.h"

std::variant<std::string, double, std::monostate> AstPrinter::print(Expr& expr)
{
	return expr.accept(*this);
}

std::variant<std::string, double, std::monostate> AstPrinter::visitBinaryExpr(Binary& expr)
{
	std::cout << "Binary: " << std::get<std::string>(parenthesize(expr.op->lexeme, *expr.left, *expr.right)) << std::endl;
	return parenthesize(expr.op->lexeme, *expr.left, *expr.right);
}

std::variant<std::string, double, std::monostate> AstPrinter::visitGroupingExpr(Grouping& expr)
{
	std::cout << "Grouping: " << std::get<std::string>(parenthesize("group",  *expr.expression)) << std::endl;
	return parenthesize("group", *expr.expression);
}

std::variant<std::string, double, std::monostate> AstPrinter::visitLiteralExpr(Literal& expr)
{
	if (expr.value->IsNull()) return "nil";
	std::cout << "Literal: " << expr.value->GetString() << std::endl;
	return expr.value->GetString();
}

std::variant<std::string, double, std::monostate> AstPrinter::visitUnaryExpr(Unary& expr)
{
	std::cout << "Unary: " << std::get<std::string>(parenthesize(expr.op->lexeme, *expr.right)) << std::endl;
    return parenthesize(expr.op->lexeme, *expr.right);
}
