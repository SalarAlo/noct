#pragma once

#include "noct/parser/Visitor.h"

namespace Noct {

struct Statement {
	virtual ~Statement() = default;
	virtual void Accept(StatementVisitor& v) const = 0;
};

}
