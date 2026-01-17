#pragma once

#include "noct/Context.h"
#include "noct/Token.h"
#include "noct/TokenType.h"
#include <string>
#include <variant>
#include <vector>

namespace Noct {

class Scanner {
public:
	Scanner(std::string_view source, Context& context)
	    : m_Source(source)
	    , m_Context(context) { };

	const std::vector<Token>& ScanTokens();

private:
	void ScanToken();
	void AddToken(TokenType type, NoctLiteral noctObj = std::monostate {});

	bool IsAtEnd() const;
	bool Match(char match);

	static bool IsDigit(char c);
	static bool IsAlpha(char c);
	static bool IsAlphaNumeric(char c);

	void HandleString();
	void HandleNumber();
	void HandleIdentifier();
	void HandleMultiLineComment();

	std::string CurrentSubstring() const;

	char Peek() const;
	char PeekNext() const;
	char Advance();

private:
	const std::string m_Source {};
	Context& m_Context;
	std::vector<Token> m_Tokens {};

	size_t m_Start {};
	size_t m_Current {};
	size_t m_Line { 1 };
};

}
