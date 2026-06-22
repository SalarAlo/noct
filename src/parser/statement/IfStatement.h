#pragma once

#include "parser/expression/ExpressionFwd.h"

#include "parser/statement/StatementFwd.h"

namespace Noct {

struct IfStatement {
	StatementPtr TrueStatement;
	ExpressionPtr Condition;
	StatementPtr FalseStatement;
};

}
