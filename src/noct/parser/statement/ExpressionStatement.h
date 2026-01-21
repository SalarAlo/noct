#pragma once

#include <memory>

#include "noct/parser/expression/Expression.h"
#include "noct/parser/statement/Statement.h"
#include "noct/parser/Visitor.h"

namespace Noct {

struct ExpressionStatement final : public Statement {
	ExpressionStatement(std::unique_ptr<Expression> expr)
	    : Expr(std::move(expr)) { }

	std::unique_ptr<Expression> Expr;

	void Accept(StatementVisitor& v) const override {
		v.Visit(*this);
	};
};

}
