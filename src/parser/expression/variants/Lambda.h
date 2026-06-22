#pragma once

#include "lexer/Token.h"

#include "parser/statement/StatementFwd.h"

namespace Noct {

struct Lambda {
	std::vector<Token> Parameters;
	StatementPtrVector Body;
	size_t FrameSize;
	size_t ClosureSize { 0 };
};

}
