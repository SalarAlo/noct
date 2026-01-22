#pragma once

#include <memory>
#include <vector>

#include "noct/parser/statement/Statement.h"
#include "noct/parser/Visitor.h"

namespace Noct {

struct BlockStatement final : public Statement {
	BlockStatement(std::vector<std::unique_ptr<Statement>> statements)
	    : Statements(std::move(statements)) { }

	std::vector<std::unique_ptr<Statement>> Statements;

	void Accept(StatementVisitor& v) const override {
		v.Visit(*this);
	};
};

}
