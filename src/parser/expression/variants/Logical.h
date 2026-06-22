#pragma once

#include "lexer/Token.h"

#include "parser/expression/ExpressionFwd.h"

namespace Noct {
struct Logical {
	ExpressionPtr Left { nullptr };
	Token Operator;
	ExpressionPtr Right { nullptr };
};

}
