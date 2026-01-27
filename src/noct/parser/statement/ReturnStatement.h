#pragma once

#include "noct/lexer/Token.h"

#include "noct/parser/expression/ExpressionFwd.h"

namespace Noct {

struct ReturnStatement {
	Token ReturnToken;
	ExpressionPtr ReturnExpr;
};

}
