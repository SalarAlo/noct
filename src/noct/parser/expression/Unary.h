#pragma once

#include "noct/lexer/Token.h"
#include "noct/lexer/TokenType.h"

#include "noct/parser/expression/ExpressionFwd.h"

namespace Noct {
struct Unary {
	Token Operator { TokenType::Nil, "", 0 };
	ExpressionPtr Right { nullptr };
};

}
