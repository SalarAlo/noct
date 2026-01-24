#pragma once

#include "noct/parser/expression/Expression.h"
#include <vector>

namespace Noct {

struct Call {
	ExpressionPtr Callee;
	std::vector<ExpressionPtr> Arguments {};
};

}
