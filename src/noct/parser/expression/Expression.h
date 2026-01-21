#pragma once

#include "noct/parser/Visitor.h"

namespace Noct {

struct Expression {
	virtual ~Expression() = default;
	virtual void Accept(ExpressionVisitor& v) const = 0;
};

}
