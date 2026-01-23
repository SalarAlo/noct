#pragma once

#include "noct/parser/expression/Expression.h"
#include "noct/lexer/Token.h"

namespace Noct {

struct Binary {
	ExpressionPtr Left;
	Token Operator;
	ExpressionPtr Right;
};

}
