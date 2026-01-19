
#pragma once

#include <memory>

#include "noct/parser/Expression.h"
#include "noct/parser/Visitor.h"

namespace Noct {
struct Ternary : Expression {
	Ternary(std::unique_ptr<Expression> c, std::unique_ptr<Expression> l, std::unique_ptr<Expression> r)
	    : Left(std::move(l))
	    , Right(std::move(r))
	    , Condition(std::move(c)) { }

	std::unique_ptr<Expression> Left { nullptr };
	std::unique_ptr<Expression> Right { nullptr };
	std::unique_ptr<Expression> Condition { nullptr };

	void Accept(Visitor& v) const override { v.Visit(*this); }
};

}
