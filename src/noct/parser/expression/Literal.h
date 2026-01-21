#pragma once

#include "noct/parser/expression/Expression.h"
#include "noct/parser/Visitor.h"
#include "noct/lexer/Token.h"

namespace Noct {
struct Literal : Expression {
	Literal(const NoctLiteral& value)
	    : Value(value) { }
	NoctLiteral Value { std::monostate {} };

	void Accept(ExpressionVisitor& v) const override { v.Visit(*this); }
};

}
