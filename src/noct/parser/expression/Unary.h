#pragma once

#include <memory>

#include "noct/parser/expression/Expression.h"

#include "noct/lexer/Token.h"
#include "noct/lexer/TokenType.h"

namespace Noct {
struct Unary {
	Token Operator { TokenType::Nil, "", 0 };
	std::unique_ptr<Expression> Right { nullptr };
};

}
