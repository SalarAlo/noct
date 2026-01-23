#pragma once

#include <memory>

#include "noct/parser/expression/Expression.h"
#include "noct/parser/statement/StatementFwd.h"

namespace Noct {

struct IfStatement {
	std::unique_ptr<Statement> TrueStatement;
	std::unique_ptr<Expression> Condition;
	std::unique_ptr<Statement> FalseStatement;
};

}
