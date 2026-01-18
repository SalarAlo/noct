#pragma once

#include <memory>

#include "noct/parser/Expression.h"
#include "noct/parser/Visitor.h"

namespace Noct {
struct Grouping : Expression {
	Grouping(std::unique_ptr<Expression> expr)
	    : GroupExpr(std::move(expr)) { }

	std::unique_ptr<Expression> GroupExpr {};
	void Accept(Visitor& v) const override { v.Visit(*this); }
};

}
