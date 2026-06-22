#include <sstream>

#include "lexer/Token.h"

namespace Noct {

std::string Token::ToString() const {
	std::ostringstream oss;
	oss << static_cast<int>(Type) << " \"" << Lexeme << "\" at line " << Line;
	return oss.str();
}

}
