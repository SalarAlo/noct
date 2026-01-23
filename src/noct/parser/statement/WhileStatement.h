
#pragma once

#include "noct/parser/expression/Expression.h"
#include "noct/parser/statement/StatementFwd.h"

namespace Noct {

struct WhileStatement {
	ExpressionPtr Condition;
	StatementPtr LoopGuts;
};

}
