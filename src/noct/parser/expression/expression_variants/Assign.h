#pragma once

#include "noct/lexer/Token.h"

#include "noct/parser/expression/ExpressionFwd.h"

namespace Noct {

struct Assign {
	Token Name;
	ExpressionPtr Value;

	size_t Slot;
	size_t Depth;
};

}
