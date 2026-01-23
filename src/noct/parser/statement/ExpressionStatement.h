#pragma once

#include <memory>

#include "noct/parser/expression/ExpressionDef.h"

namespace Noct {

struct ExpressionStatement {
	std::unique_ptr<Expression> Expr;
};

}
