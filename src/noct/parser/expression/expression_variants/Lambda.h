#pragma once

#include "noct/lexer/Token.h"

#include "noct/parser/statement/StatementFwd.h"

namespace Noct {

struct Lambda {
	std::vector<Token> Parameters;
	StatementPtrVector Body;
	size_t LocalCount;
};

}
