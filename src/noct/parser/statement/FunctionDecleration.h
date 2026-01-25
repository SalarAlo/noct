#pragma once

#include <vector>

#include "noct/lexer/Token.h"

#include "noct/parser/statement/StatementFwd.h"

namespace Noct {

struct FunctionDecleration {
	Token Name;
	std::vector<Token> ArgumentNames;
	StatementPtrVector Body;
};

}
