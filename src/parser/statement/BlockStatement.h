#pragma once

#include "parser/statement/StatementFwd.h"

namespace Noct {

struct BlockStatement {
	StatementPtrVector Statements;
	size_t FrameSize;
};

}
