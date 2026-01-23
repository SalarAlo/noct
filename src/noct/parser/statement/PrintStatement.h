#pragma once

#include <memory>

#include "noct/parser/expression/Expression.h"
#include "noct/parser/expression/ExpressionDef.h"

namespace Noct {

struct PrintStatement {
	std::unique_ptr<Expression> PrintExpr;
};

}
