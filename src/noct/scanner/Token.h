#pragma once

#include <string>
#include <variant>
#include <cstddef>

#include "noct/scanner/TokenType.h"

namespace Noct {

using NoctLiteral = std::variant<std::monostate, double, std::string, bool>;

struct Token {
	Token(TokenType type, std::string_view lexeme, NoctLiteral literal, std::size_t line)
	    : Type(type)
	    , Lexeme(lexeme)
	    , Literal(literal)
	    , Line(line) { };

	TokenType Type;
	std::string Lexeme;
	NoctLiteral Literal;
	std::size_t Line;

	std::string ToString() const;
};

}
