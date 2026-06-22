#pragma once

#include "lexer/Token.h"
#include "lexer/TokenType.h"

#include "parser/expression/ExpressionFwd.h"

namespace Noct {
struct Unary {
	Token Operator { TokenType::Nil, "", 0 };
	ExpressionPtr Right { nullptr };
};

}
