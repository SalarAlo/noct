#pragma once

#include <vector>

#include "noct/lexer/Token.h"

#include "noct/parser/expression/ExpressionFwd.h"

namespace Noct {

struct Call {
	ExpressionPtr Callee;
	std::vector<ExpressionPtr> Arguments {};
	Token Paren;
};

}
