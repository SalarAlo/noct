#include <fmt/format.h>

#include "noct/lexer/Lexer.h"

#include "noct/lexer/Keywords.h"
#include "noct/lexer/TokenType.h"

namespace Noct {

const std::vector<Token>& Lexer::ScanTokens() {
	while (!IsAtEnd()) {
		m_Start = m_Current;
		ScanToken();
	}

	m_Tokens.emplace_back(TokenType::Eof, "", m_Line);
	return m_Tokens;
}

void Lexer::ScanToken() {
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
	case ';':
		AddToken(TokenType::Semicolon);
		break;
	case '*':
		AddToken(TokenType::Star);
		break;
	case '?':
		AddToken(TokenType::QuestionMark);
		break;
	case ':':
		AddToken(TokenType::Colon);
		break;
	case '%':
		AddToken(TokenType::Percentage);
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
	case '-':
		AddToken(Match('-') ? TokenType::MinusMinus : TokenType::Minus);
		break;
	case '+':
		AddToken(Match('+') ? TokenType::PlusPlus : TokenType::Plus);
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
			m_Context.ReportParseError(m_Line, fmt::format("Unexpected character: '{}'", currentCharacter));
		}
		break;
	}
#pragma endregion
	}
}

char Lexer::Peek() const {
	if (IsAtEnd())
		return 0;

	return m_Source[m_Current];
}
char Lexer::PeekNext() const {
	if (m_Current + 1 >= m_Source.size())
		return 0;

	return m_Source[m_Current + 1];
}

char Lexer::Advance() {
	return m_Source[m_Current++];
}

void Lexer::HandleString() {
	while (!IsAtEnd() && Peek() != '"') {
		// supporting multi line strings
		if (Peek() == '\n') {
			m_Line++;
		}

		// skip over '"'
		Advance();
	}

	if (IsAtEnd()) {
		m_Context.ReportParseError(m_Line, "Unterminated string");
	}

	Advance();

	std::string value { m_Source.substr(m_Start + 1, (m_Current - m_Start) - 2) };
	AddToken(TokenType::String);
}

void Lexer::HandleNumber() {
	while (IsDigit(Peek()))
		Advance();

	if (Peek() == '.' && IsDigit(PeekNext())) {
		Advance();
		while (IsDigit(Peek()))
			Advance();
	}

	AddToken(TokenType::Number);
}

void Lexer::HandleIdentifier() {
	while (IsAlphaNumeric(Peek()))
		Advance();

	std::string lexeme { CurrentSubstring() };
	if (Keywords.find(lexeme) != Keywords.end()) {
		AddToken(Keywords.at(lexeme));
		return;
	}

	AddToken(TokenType::Identifier);
}

void Lexer::HandleMultiLineComment() {
	while (!IsAtEnd()) {
		if (Peek() == '*' && PeekNext() == '/')
			break;

		if (Peek() == '\n')
			++m_Line;

		Advance();
	}

	if (IsAtEnd()) {
		m_Context.ReportParseError(m_Line, "Unterminated multi-line comment");
		return;
	}

	// skip over "*/"
	Advance();
	Advance();
}

bool Lexer::IsDigit(char c) {
	return c <= '9' && c >= '0';
}

bool Lexer::IsAlpha(char c) {
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

bool Lexer::IsAlphaNumeric(char c) {
	return IsAlpha(c) || IsDigit(c);
}

bool Lexer::IsAtEnd() const {
	return m_Current >= m_Source.length();
}

bool Lexer::Match(char match) {
	if (IsAtEnd())
		return false;

	if (m_Source[m_Current] != match)
		return false;

	m_Current++;
	return true;
}

void Lexer::AddToken(TokenType type) {
	auto lexeme { CurrentSubstring() };
	m_Tokens.emplace_back(type, lexeme, m_Line);
}

std::string Lexer::CurrentSubstring() const {
	return m_Source.substr(m_Start, m_Current - m_Start);
}

}
