#include "noct/Context.h"

#include "noct/Logger.h"

namespace Noct {

void Context::ReportParseError(std::size_t line, std::string_view message, std::string_view where) {
	Logger::Error("[line {}] Error{}: {}", line, where, message);
	HadParseError = true;
}

void Context::ReportParseError(const Token& token, std::string_view message) {
	if (token.Type == TokenType::Eof) {
		ReportParseError(token.Line, message, " at end");
	} else {
		ReportParseError(
		    token.Line,
		    message,
		    std::string(" at '") + token.Lexeme + "'");
	}
}

void Context::ReportRuntimeError(const RuntimeError& err) {
	const Token& token = err.Where();

	if (token.Type == TokenType::Eof) {
		Logger::Error(
		    "[line {}] Runtime error at end: {}",
		    token.Line,
		    err.what());
	} else {
		Logger::Error(
		    "[line {}] Runtime error at '{}': {}",
		    token.Line,
		    token.Lexeme,
		    err.what());
	}

	HadRuntimeError = true;
}

void Context::ReportResolveError(const Token& token, std::string_view message) {
	if (token.Type == TokenType::Eof) {
		ReportResolveError(token.Line, message, " at end");
	} else {
		ReportResolveError(token.Line, message, std::string(" at '") + token.Lexeme + "'");
	}
}

void Context::ReportResolveError(std::size_t line, std::string_view message, std::string_view where) {
	Logger::Error("[line {}] Error{}: {}", line, where, message);
	HadSemanticError = true;
}

}
