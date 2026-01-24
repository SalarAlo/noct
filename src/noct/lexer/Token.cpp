#include "noct/lexer/Token.h"
#include <sstream>

namespace Noct {

std::string Token::ToString() const {
	std::ostringstream oss;
	oss << static_cast<int>(Type) << " \"" << Lexeme << "\" at line " << Line;
	return oss.str();
}

}
