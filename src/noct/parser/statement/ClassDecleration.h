#pragma once

#include <vector>

#include "noct/lexer/Token.h"

#include "noct/parser/statement/FunctionDecleration.h"

namespace Noct {

struct ClassDecleration {
	Token Name;
	std::vector<FunctionDecleration> Methods;
	size_t Slot;
};

}
