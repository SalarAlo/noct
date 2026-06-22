#pragma once

#include "lexer/Token.h"

#include "parser/expression/ExpressionFwd.h"

namespace Noct {

struct ReturnStatement {
	Token ReturnToken;
	ExpressionPtr ReturnExpr;
};

}
