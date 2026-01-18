#pragma once

#include "Visitor.h"

namespace Noct {

struct Expression {
	virtual ~Expression() = default;
	virtual void Accept(Visitor& v) const = 0;
};

}
