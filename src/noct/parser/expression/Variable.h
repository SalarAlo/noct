#pragma once

#include "noct/lexer/Token.h"
#include "noct/lexer/TokenType.h"

namespace Noct {
struct Variable {
	Token Name { TokenType::Nil, "", std::monostate {}, 0 };
};

}
