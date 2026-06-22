#pragma once

#include <span>
#include <filesystem>
#include <string_view>
#include <vector>

#include "Diagnostic.h"
#include "exceptions/RuntimeException.h"

#include "lexer/Token.h"

namespace Noct {

struct Context {
	std::span<char*> Args;
	std::filesystem::path SourcePath {};
	bool LoggingEnabled { true };

	bool HadParseError { false };
	bool HadRuntimeError { false };
	bool HadSemanticError { false };
	std::vector<NoctDiagnostic> Diagnostics {};

	void ReportParseError(std::size_t line, std::string_view message, std::string_view where = "");
	void ReportParseError(const Token& token, std::string_view message);

	void ReportRuntimeError(const RuntimeError& err);

	void ReportResolveError(std::size_t line, std::string_view message, std::string_view where = "");
	void ReportResolveError(const Token& token, std::string_view message);

private:
	void AddDiagnostic(NoctSeverity severity, std::size_t line, std::string_view message, std::string code = {});
};

}
