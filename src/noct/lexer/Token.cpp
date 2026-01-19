#include "noct/lexer/Token.h"
#include <sstream>

namespace Noct {

std::string Token::ToString() const {
	std::ostringstream out;
	out << static_cast<int>(Type) << " " << Lexeme << " ";

	std::visit(
	    [&](const auto& value) constexpr {
		    using T = std::decay_t<decltype(value)>;
		    if constexpr (std::is_same_v<T, std::monostate>) {
			    out << "nil";
		    } else {
			    out << value;
		    }
	    },
	    Literal);

	return out.str();
}

}
