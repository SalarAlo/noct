#pragma once

#include "lexer/Token.h"

#include "parser/expression/ExpressionFwd.h"

namespace Noct {

struct VariableDecleration {
	Token Name;
	ExpressionPtr Initialiser;
	size_t Slot;
};

}
