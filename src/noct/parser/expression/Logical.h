#pragma once

#include <memory>

#include "noct/parser/expression/Expression.h"

#include "noct/lexer/Token.h"

namespace Noct {
struct Logical {
	std::unique_ptr<Expression> Left { nullptr };
	Token Operator;
	std::unique_ptr<Expression> Right { nullptr };
};

}
