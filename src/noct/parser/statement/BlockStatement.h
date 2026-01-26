#pragma once

#include "noct/parser/statement/StatementFwd.h"

namespace Noct {

struct BlockStatement {
	StatementPtrVector Statements;
	size_t LocalCount;
};

}
