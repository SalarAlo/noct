#pragma once

#include "lexer/Token.h"

#include "parser/expression/ExpressionFwd.h"

namespace Noct {

struct Set {
	Token Name;
	Token Operator;
	ExpressionPtr Instance;
	ExpressionPtr Value;
};

}
