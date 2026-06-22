#pragma once

#include <cstddef>

#include "lexer/Token.h"

#include "parser/expression/variants/UnresolvedType.h"

namespace Noct {

struct Variable {
	Token Name;
	size_t Slot { UNRESOLVED };
	size_t Depth { UNRESOLVED };
};

}
