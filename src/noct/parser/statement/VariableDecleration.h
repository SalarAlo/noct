#pragma once

#include <memory>

#include "noct/lexer/Token.h"
#include "noct/parser/expression/Expression.h"
#include "noct/parser/statement/Statement.h"
#include "noct/parser/Visitor.h"

namespace Noct {

struct VariableDecleration final : public Statement {
	VariableDecleration(const Token& name, std::unique_ptr<Expression> expr)
	    : Name(name)
	    , Initialiser(std::move(expr)) { }

	Token Name;
	std::unique_ptr<Expression> Initialiser;

	void Accept(StatementVisitor& v) const override {
		v.Visit(*this);
	};
};

}
