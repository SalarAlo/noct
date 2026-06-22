#pragma once

#include "parser/expression/ExpressionFwd.h"

#include "parser/statement/StatementFwd.h"

namespace Noct {

struct WhileStatement {
	ExpressionPtr Condition;
	StatementPtr LoopGuts;
};

}
