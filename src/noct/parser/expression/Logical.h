#pragma once

#include <memory>

#include "noct/parser/expression/Expression.h"
#include "noct/parser/Visitor.h"

#include "noct/lexer/Token.h"
#include "noct/lexer/TokenType.h"

namespace Noct {
struct Logical : Expression {
	Logical(std::unique_ptr<Expression> l, const Token& op, std::unique_ptr<Expression> r)
	    : Left(std::move(l))
	    , Operator(op)
	    , Right(std::move(r)) { }

	std::unique_ptr<Expression> Left { nullptr };
	std::unique_ptr<Expression> Right { nullptr };
	Token Operator { TokenType::Nil, "", std::monostate {}, 0 };

	void Accept(ExpressionVisitor& v) const override { v.Visit(*this); }
};

}
