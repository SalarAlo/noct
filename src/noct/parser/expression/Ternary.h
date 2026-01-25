#pragma once

#include "noct/parser/expression/ExpressionFwd.h"

namespace Noct {
struct Ternary {
	ExpressionPtr Left { nullptr };
	ExpressionPtr Right { nullptr };
	ExpressionPtr Condition { nullptr };
};

}
