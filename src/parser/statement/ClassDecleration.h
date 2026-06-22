#pragma once

#include <vector>

#include "lexer/Token.h"

#include "parser/statement/FunctionDecleration.h"

namespace Noct {

struct ClassDecleration {
	Token Name;
	std::vector<FunctionDecleration> Methods;
	size_t Slot;
};

}
