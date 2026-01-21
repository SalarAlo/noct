#pragma once

#include "noct/parser/expression/Expression.h"

#include "noct/lexer/Token.h"
#include "noct/lexer/TokenType.h"

namespace Noct {
struct Variable : Expression {
	Variable(const Token& op)
	    : Name(op) {
	    };

	Token Name { TokenType::Nil, "", std::monostate {}, 0 };

	void Accept(ExpressionVisitor& v) const override { v.Visit(*this); }
};

}
