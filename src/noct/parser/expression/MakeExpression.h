#pragma once
#include <memory>
#include <utility>

#include "noct/parser/expression/Expression.h"

namespace Noct {

template <typename T, typename... Args>
ExpressionPtr make_expression(Args&&... args) {
	return std::make_unique<Expression>(T { std::forward<Args>(args)... });
}

}
