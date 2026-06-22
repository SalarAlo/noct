#pragma once

#include "lexer/Token.h"

#include "parser/expression/ExpressionFwd.h"

namespace Noct {

struct Binary {
	ExpressionPtr Left;
	Token Operator;
	ExpressionPtr Right;
};

}
