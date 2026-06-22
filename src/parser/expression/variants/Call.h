#pragma once

#include <vector>

#include "lexer/Token.h"

#include "parser/expression/ExpressionFwd.h"

namespace Noct {

struct Call {
	ExpressionPtr Callee;
	std::vector<ExpressionPtr> Arguments {};
	Token Paren;
};

}
