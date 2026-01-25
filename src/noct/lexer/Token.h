#pragma once

#include <cstddef>

#include <string>

#include "noct/lexer/TokenType.h"

namespace Noct {

struct Token {
	Token(TokenType type, std::string_view lexeme, std::size_t line)
	    : Type(type)
	    , Lexeme(lexeme)
	    , Line(line) { };

	TokenType Type;
	std::string Lexeme;
	std::size_t Line;

	std::string ToString() const;
};

inline bool operator==(const Token& a, const Token& b) {
	return a.Type == b.Type
	    && a.Lexeme == b.Lexeme;
}

inline bool operator!=(const Token& a, const Token& b) {
	return !(a == b);
}

}
