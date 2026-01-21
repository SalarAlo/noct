#pragma once

#include <memory>

#include "noct/parser/expression/Expression.h"

#include "noct/lexer/Token.h"
#include "noct/lexer/TokenType.h"

namespace Noct {
struct Assign : Expression {
	Assign(const Token& name, std::unique_ptr<Expression> v)
	    : name(name)
	    , value(std::move(v)) {
	    };

	Token name { TokenType::Nil, "", std::monostate {}, 0 };
	std::unique_ptr<Expression> value { nullptr };

	void Accept(ExpressionVisitor& v) const override { v.Visit(*this); }
};

}
