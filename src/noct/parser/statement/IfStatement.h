#pragma once

#include <memory>

#include "noct/parser/expression/Expression.h"
#include "noct/parser/statement/Statement.h"
#include "noct/parser/Visitor.h"

namespace Noct {

struct IfStatement final : public Statement {
	IfStatement(
	    std::unique_ptr<Expression> condition,
	    std::unique_ptr<Statement> trueStatement,
	    std::unique_ptr<Statement> falseStatement = nullptr)
	    : Condition(std::move(condition))
	    , TrueStatement(std::move(trueStatement))
	    , FalseStatement(std::move(falseStatement)) { }

	std::unique_ptr<Expression> Condition;
	std::unique_ptr<Statement> TrueStatement;
	std::unique_ptr<Statement> FalseStatement;

	void Accept(StatementVisitor& v) const override {
		v.Visit(*this);
	};
};

}
