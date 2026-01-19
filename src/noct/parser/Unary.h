#pragma once

#include <memory>

#include "noct/parser/Expression.h"

#include "noct/lexer/Token.h"
#include "noct/lexer/TokenType.h"

namespace Noct {
struct Unary : Expression {
	Unary(const Token& op, std::unique_ptr<Expression> r)
	    : Operator(op)
	    , Right(std::move(r)) {

	    };

	Token Operator { TokenType::Nil, "", std::monostate {}, 0 };
	std::unique_ptr<Expression> Right { nullptr };

	void Accept(Visitor& v) const override { v.Visit(*this); }
};

}
