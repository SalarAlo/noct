#pragma once

#include "noct/lexer/Token.h"

namespace Noct {

struct This {
	Token Keyword;
	size_t Slot;
	size_t Depth;
};

}
