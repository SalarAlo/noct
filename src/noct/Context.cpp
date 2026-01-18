#include "noct/Context.h"
#include "noct/Logger.h"

namespace Noct {

void Context::RegisterSourceCodeError(std::size_t line, std::string_view msg, std::string_view where) {
	Logger::Error("[line {}] {}: {}", line, where, msg);

	HadError = true;
}

void Context::RegisterTokenError(Token token, std::string_view msg) {
	if (token.Type == TokenType::Eof) {
		RegisterSourceCodeError(token.Line, " at end", msg);
	} else {
		RegisterSourceCodeError(token.Line, " at '" + token.Lexeme + "'", msg);
	}

	HadError = true;
}

}
