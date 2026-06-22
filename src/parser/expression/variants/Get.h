#pragma once

#include "lexer/Token.h"

#include "parser/expression/ExpressionFwd.h"

namespace Noct {

struct Get {
	ExpressionPtr Instance;
	Token Name;
};

}
