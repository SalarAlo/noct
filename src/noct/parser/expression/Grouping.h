#pragma once

#include <memory>

#include "noct/parser/expression/Expression.h"

namespace Noct {
struct Grouping {
	std::unique_ptr<Expression> GroupExpr {};
};

}
