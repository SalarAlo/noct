
#pragma once

#include <memory>

#include "noct/parser/expression/Expression.h"
#include "noct/parser/statement/Statement.h"
#include "noct/parser/Visitor.h"

namespace Noct {

struct WhileStatement final : public Statement {
	WhileStatement(
	    std::unique_ptr<Expression> condition,
	    std::unique_ptr<Statement> execStatement)
	    : RunCondition(std::move(condition))
	    , ExecuteStatement(std::move(execStatement)) { }

	std::unique_ptr<Expression> RunCondition;
	std::unique_ptr<Statement> ExecuteStatement;

	void Accept(StatementVisitor& v) const override {
		v.Visit(*this);
	};
};

}
