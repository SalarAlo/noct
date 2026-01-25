#pragma once

#include "noct/lexer/Token.h"

#include "noct/parser/expression/ExpressionFwd.h"

namespace Noct {

struct Binary {
	ExpressionPtr Left;
	Token Operator;
	ExpressionPtr Right;
};

}
