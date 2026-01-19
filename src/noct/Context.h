
#pragma once

#include <span>
#include <string_view>

#include "noct/lexer/Token.h"

namespace Noct {

struct Context {
	std::span<char*> Args;
	bool HadError { false };

	void RegisterSourceCodeError(std::size_t line, std::string_view msg, std::string_view where = "");
	void RegisterTokenError(Token token, std::string_view msg);
};

}
