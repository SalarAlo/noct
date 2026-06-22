#pragma once

#include <vector>

#include "lexer/Token.h"

#include "parser/statement/StatementFwd.h"

namespace Noct {

struct FunctionDecleration {
	Token Name;
	std::vector<Token> Parameters;
	StatementPtrVector Body;
	size_t Slot;
	size_t FrameSize;
	size_t ClosureSize {};
};

}
