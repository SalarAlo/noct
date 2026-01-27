#pragma once

#include <stdexcept>
#include <string>

#include "noct/lexer/Token.h"

namespace Noct {

class RuntimeError : public std::runtime_error {
public:
	RuntimeError(std::string msg)
	    : std::runtime_error(std::move(msg))
	    , m_Token(TokenType::Eof, "", 0) { }

	RuntimeError(const Token& where, std::string msg)
	    : std::runtime_error(std::move(msg))
	    , m_Token(where) { }

	const Token& Where() const { return m_Token; }

private:
	Token m_Token;
};

}
