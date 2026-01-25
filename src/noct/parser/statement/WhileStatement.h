#pragma once

#include "noct/parser/expression/ExpressionFwd.h"

#include "noct/parser/statement/StatementFwd.h"

namespace Noct {

struct WhileStatement {
	ExpressionPtr Condition;
	StatementPtr LoopGuts;
};

}
