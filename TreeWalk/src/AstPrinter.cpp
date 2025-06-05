#include "AstPrinter.h"

std::variant<std::string, double, std::monostate> AstPrinter::print(Expr& expr)
{
	return expr.accept(*this);
}

std::variant<std::string, double, std::monostate> AstPrinter::visitBinaryExpr(Binary& expr)
{
	return parenthesize(expr.op->lexeme, *expr.left, *expr.right);
}

std::variant<std::string, double, std::monostate> AstPrinter::visitGroupingExpr(Grouping& expr)
{
	return parenthesize("group", *expr.expression);
}

std::variant<std::string, double, std::monostate> AstPrinter::visitLiteralExpr(Literal& expr)
{
	if (expr.value->IsNull()) return "nil";
	return expr.value->GetString();
}

std::variant<std::string, double, std::monostate> AstPrinter::visitUnaryExpr(Unary& expr)
{
    return parenthesize(expr.op->lexeme, *expr.right);
}
