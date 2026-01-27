#pragma once

#include <span>
#include <string_view>

#include "noct/exceptions/RuntimeException.h"

#include "noct/lexer/Token.h"

namespace Noct {

struct Context {
	std::span<char*> Args;
	bool LoggingEnabled { true };

	bool HadParseError { false };
	bool HadRuntimeError { false };
	bool HadSemanticError { false };

	void ReportParseError(std::size_t line, std::string_view message, std::string_view where = "");
	void ReportParseError(const Token& token, std::string_view message);

	void ReportRuntimeError(const RuntimeError& err);

	void ReportResolveError(std::size_t line, std::string_view message, std::string_view where = "");
	void ReportResolveError(const Token& token, std::string_view message);
};

}
