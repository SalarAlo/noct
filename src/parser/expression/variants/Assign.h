#pragma once

#include "lexer/Token.h"

#include "parser/expression/ExpressionFwd.h"

#include "parser/expression/variants/UnresolvedType.h"

namespace Noct {

struct Assign {
	Token Name;
	Token Operator;
	ExpressionPtr Value;

	size_t Slot { UNRESOLVED };
	size_t Depth { UNRESOLVED };
};

}
