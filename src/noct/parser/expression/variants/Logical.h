#pragma once

#include "noct/lexer/Token.h"

#include "noct/parser/expression/ExpressionFwd.h"

namespace Noct {
struct Logical {
	ExpressionPtr Left { nullptr };
	Token Operator;
	ExpressionPtr Right { nullptr };
};

}
