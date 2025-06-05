#include "AstPrinter.h"

Object AstPrinter::print(Expr& expr)
{
	return expr.accept(*this);
}

Object AstPrinter::visitBinaryExpr(Binary& expr)
{
	return parenthesize(expr.op->lexeme, *expr.left, *expr.right);
}

Object AstPrinter::visitGroupingExpr(Grouping& expr)
{
	return parenthesize("group", *expr.expression);
}

Object AstPrinter::visitLiteralExpr(Literal& expr)
{
	if (expr.value->IsNull()) return "nil";
	return expr.value->GetString();
}

Object AstPrinter::visitUnaryExpr(Unary& expr)
{
    return parenthesize(expr.op->lexeme, *expr.right);
}
