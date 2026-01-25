#pragma once

#include "noct/parser/expression/ExpressionFwd.h"

#include "noct/parser/statement/StatementFwd.h"

namespace Noct {

struct IfStatement {
	StatementPtr TrueStatement;
	ExpressionPtr Condition;
	StatementPtr FalseStatement;
};

}
