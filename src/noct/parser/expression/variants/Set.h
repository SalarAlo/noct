#pragma once

#include "noct/lexer/Token.h"

#include "noct/parser/expression/ExpressionFwd.h"

namespace Noct {

struct Set {
	Token Name;
	ExpressionPtr Instance;
	ExpressionPtr Value;
};

}
