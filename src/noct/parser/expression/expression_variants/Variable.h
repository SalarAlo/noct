#pragma once

#include <cstddef>

#include "noct/lexer/Token.h"

namespace Noct {
struct Variable {
	Token Name;
	size_t Slot {};
	size_t Depth {};
};

}
