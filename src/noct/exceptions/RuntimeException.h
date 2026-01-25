#pragma once

#include <exception>
#include <string>

#include "noct/lexer/Token.h"

namespace Noct {

class RuntimeError : public std::exception {
public:
	explicit RuntimeError(const Token& token, std::string_view msg)
	    : m_Message(msg)
	    , m_Token(token) {
	}

	const char* what() const noexcept override {
		return m_Message.c_str();
	}

	Token GetToken() const { return m_Token; }

private:
	std::string m_Message;
	Token m_Token;
};

}
