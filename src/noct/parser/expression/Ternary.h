
#pragma once

#include <memory>

#include "noct/parser/expression/Expression.h"

namespace Noct {
struct Ternary {
	std::unique_ptr<Expression> Left { nullptr };
	std::unique_ptr<Expression> Right { nullptr };
	std::unique_ptr<Expression> Condition { nullptr };
};

}
