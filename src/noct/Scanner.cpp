#include "noct/Scanner.h"

#include <charconv>

#include "noct/TokenType.h"
#include "noct/Keywords.h"

namespace Noct {

const std::vector<Token>& Scanner::ScanTokens() {
	while (!IsAtEnd()) {
		m_Start = m_Current;
		ScanToken();
	}

	m_Tokens.emplace_back(TokenType::Eof, "", std::monostate {}, m_Line);
	return m_Tokens;
}

void Scanner::ScanToken() {
	char currentCharacter { Advance() };
	switch (currentCharacter) {
#pragma region Single Character Tokens
	case '(':
		AddToken(TokenType::LeftParen);
		break;
	case ')':
		AddToken(TokenType::RightParen);
		break;
	case '{':
		AddToken(TokenType::LeftBrace);
		break;
	case '}':
		AddToken(TokenType::RightBrace);
		break;
	case ',':
		AddToken(TokenType::Comma);
		break;
	case '.':
		AddToken(TokenType::Dot);
		break;
	case '-':
		AddToken(TokenType::Minus);
		break;
	case '+':
		AddToken(TokenType::Plus);
		break;
	case ';':
		AddToken(TokenType::Semicolon);
		break;
	case '*':
		AddToken(TokenType::Star);
		break;
#pragma endregion
#pragma region Multi Character Tokens
	case '!':
		AddToken(Match('=') ? TokenType::BangEqual : TokenType::Bang);
		break;
	case '=':
		AddToken(Match('=') ? TokenType::EqualEqual : TokenType::Equal);
		break;
	case '<':
		AddToken(Match('=') ? TokenType::LessEqual : TokenType::Less);
		break;
	case '>':
		AddToken(Match('=') ? TokenType::GreaterEqual : TokenType::Greater);
		break;
#pragma endregion
#pragma region Other Token Types
	case '/': {
		if (Match('/')) {
			while (Peek() != '\n' && !IsAtEnd())
				Advance();
		} else if (Match('*')) {
			HandleMultiLineComment();
		} else {
			AddToken(TokenType::Slash);
		}
		break;
	}
	case '"':
		HandleString();
		break;
#pragma endregion
#pragma region Whitespace
	case ' ':
		[[fallthrough]];
	case '\r':
		[[fallthrough]];
	case '\t':
		break;
	case '\n':
		m_Line++;
		break;
#pragma endregion
#pragma region Default Case
	default: {
		if (IsDigit(currentCharacter)) {
			HandleNumber();
		} else if (IsAlpha(currentCharacter)) {
			HandleIdentifier();
		} else {
			m_Context.RegisterSourceCodeError(m_Line, "Unexpected character!");
		}
		break;
	}
#pragma endregion
	}
}

char Scanner::Peek() const {
	if (IsAtEnd())
		return 0;

	return m_Source[m_Current];
}
char Scanner::PeekNext() const {
	if (m_Current + 1 >= m_Source.size())
		return 0;

	return m_Source[m_Current + 1];
}

char Scanner::Advance() {
	return m_Source[m_Current++];
}

void Scanner::HandleString() {
	while (!IsAtEnd() && Peek() != '"') {
		// supporting multi line strings
		if (Peek() == '\n') {
			m_Line++;
		}

		// skip over '"'
		Advance();
	}

	if (IsAtEnd()) {
		m_Context.RegisterSourceCodeError(m_Line, "Unterminated String");
	}

	Advance();

	std::string value { m_Source.substr(m_Start + 1, (m_Current - m_Start) - 2) };
	AddToken(TokenType::String, value);
}

void Scanner::HandleNumber() {
	while (IsDigit(Peek()))
		Advance();
	if (Peek() == '.' && IsDigit(PeekNext())) {
		Advance();
		while (IsDigit(Peek()))
			Advance();
	}

	double value {};
	auto numberStr { CurrentSubstring() };
	auto [_, ec] = std::from_chars(numberStr.data(), numberStr.data() + numberStr.size(), value);
	if (ec != std::errc {}) {
		m_Context.RegisterSourceCodeError(m_Line, "Invalid number literal");
	}

	AddToken(TokenType::Number, value);
}

void Scanner::HandleIdentifier() {
	while (IsAlphaNumeric(Peek()))
		Advance();

	std::string lexeme { CurrentSubstring() };
	if (Keywords.find(lexeme) != Keywords.end()) {
		AddToken(Keywords.at(lexeme));
		return;
	}

	AddToken(TokenType::Identifier);
}

void Scanner::HandleMultiLineComment() {
	while (!IsAtEnd()) {
		if (Peek() == '*' && PeekNext() == '/')
			break;

		if (Peek() == '\n')
			++m_Line;

		Advance();
	}

	if (IsAtEnd()) {
		m_Context.RegisterSourceCodeError(m_Line, "Unterminated multi-line comment");
		return;
	}

	// skip over "*/"
	Advance();
	Advance();
}

bool Scanner::IsDigit(char c) {
	return c <= '9' && c >= '0';
}

bool Scanner::IsAlpha(char c) {
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

bool Scanner::IsAlphaNumeric(char c) {
	return IsAlpha(c) || IsDigit(c);
}

bool Scanner::IsAtEnd() const {
	return m_Current >= m_Source.length();
}

bool Scanner::Match(char match) {
	if (IsAtEnd())
		return false;

	if (m_Source[m_Current] != match)
		return false;

	m_Current++;
	return true;
}

void Scanner::AddToken(TokenType type, NoctLiteral noctLiteral) {
	auto lexeme { CurrentSubstring() };
	m_Tokens.emplace_back(type, lexeme, noctLiteral, m_Line);
}

std::string Scanner::CurrentSubstring() const {
	return m_Source.substr(m_Start, m_Current - m_Start);
}

}
