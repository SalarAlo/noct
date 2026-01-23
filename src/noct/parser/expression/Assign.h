#pragma once

#include "noct/parser/expression/Expression.h"

#include "noct/lexer/Token.h"

namespace Noct {

struct Assign {
	Token Name;
	ExpressionPtr Value;
};

}
