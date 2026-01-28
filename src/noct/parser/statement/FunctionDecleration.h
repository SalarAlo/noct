#pragma once

#include <vector>

#include "noct/lexer/Token.h"

#include "noct/parser/statement/StatementFwd.h"

namespace Noct {

enum class FunctionType {
	Function,
	Method,
};

struct FunctionDecleration {
	Token Name;
	std::vector<Token> Parameters;
	StatementPtrVector Body;
	size_t Slot;
	size_t LocalCount;
	FunctionType Type { FunctionType::Function };
};

}
