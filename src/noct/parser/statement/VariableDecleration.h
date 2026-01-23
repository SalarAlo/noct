#pragma once

#include <memory>

#include "noct/lexer/Token.h"
#include "noct/parser/expression/Expression.h"

namespace Noct {

struct VariableDecleration {
	Token Name;
	std::unique_ptr<Expression> Initialiser;
};

}
