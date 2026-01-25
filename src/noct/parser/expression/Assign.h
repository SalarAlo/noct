#pragma once

#include "noct/lexer/Token.h"

#include "noct/parser/expression/ExpressionFwd.h"

namespace Noct {

struct Assign {
	Token Name;
	ExpressionPtr Value;
};

}
