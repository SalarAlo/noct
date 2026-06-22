#pragma once

#include <cstddef>

#include "lexer/Token.h"

#include "parser/expression/variants/UnresolvedType.h"

namespace Noct {

struct ImportStatement {
	Token Name;
	size_t Slot { UNRESOLVED };
};

}
