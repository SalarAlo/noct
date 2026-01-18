#include "noct/parser/Parser.h"

#include <stdexcept>

#include "noct/parser/Binary.h"
#include "noct/parser/Grouping.h"
#include "noct/parser/Literal.h"
#include "noct/parser/Unary.h"

using enum Noct::TokenType;

namespace Noct {

Parser::Parser(const std::vector<Token>& tokens, Context& ctx)
    : m_Tokens(tokens)
    , m_Context(ctx) { }

std::unique_ptr<Expression> Parser::Parse() {
	try {
		auto expr = Expr();
		Consume(Eof, "Unexpected tokens after expression");
		return expr;
	} catch (const std::exception& e) {
		m_Context.RegisterSourceCodeError(0, e.what());
		return nullptr;
	}
}

std::unique_ptr<Expression> Parser::Expr() {
	return Equality();
}

std::unique_ptr<Expression> Parser::Equality() {
	auto left = Comparison();

	while (MatchAny({ BangEqual, EqualEqual })) {
		Token op = GetPrevious();
		auto right = Comparison();
		left = std::make_unique<Binary>(std::move(left), op, std::move(right));
	}

	return left;
}

std::unique_ptr<Expression> Parser::Comparison() {
	auto left = Term();

	while (MatchAny({ Greater, GreaterEqual, Less, LessEqual })) {
		Token op = GetPrevious();
		auto right = Term();
		left = std::make_unique<Binary>(std::move(left), op, std::move(right));
	}

	return left;
}

std::unique_ptr<Expression> Parser::Term() {
	auto left = Factor();

	while (MatchAny({ Minus, Plus })) {
		Token op = GetPrevious();
		auto right = Factor();
		left = std::make_unique<Binary>(std::move(left), op, std::move(right));
	}

	return left;
}

std::unique_ptr<Expression> Parser::Factor() {
	auto left = Unary();

	while (MatchAny({ Slash, Star })) {
		Token op = GetPrevious();
		auto right = Unary();
		left = std::make_unique<Binary>(std::move(left), op, std::move(right));
	}

	return left;
}

std::unique_ptr<Expression> Parser::Unary() {
	if (MatchAny({ Bang, Minus })) {
		Token op = GetPrevious();
		auto right = Unary();
		return std::make_unique<Noct::Unary>(op, std::move(right));
	}

	return Primary();
}

std::unique_ptr<Expression> Parser::Primary() {
	if (MatchAny({ False }))
		return std::make_unique<Literal>(false);

	if (MatchAny({ True }))
		return std::make_unique<Literal>(true);

	if (MatchAny({ Nil }))
		return std::make_unique<Literal>(std::monostate {});

	if (MatchAny({ Number, String }))
		return std::make_unique<Literal>(GetPrevious().Literal);

	if (MatchAny({ LeftParen })) {
		auto expr = Expr();
		Consume(RightParen, "Expect ')' after expression");
		return std::make_unique<Grouping>(std::move(expr));
	}

	m_Context.RegisterTokenError(GetCurrent(), "Expected expression");
	throw std::logic_error("Expected expression");
}

bool Parser::MatchAny(std::initializer_list<TokenType> types) {
	for (TokenType t : types) {
		if (MatchCurrent(t)) {
			Advance();
			return true;
		}
	}
	return false;
}

bool Parser::MatchCurrent(TokenType type) const {
	return !IsAtEnd() && GetCurrent().Type == type;
}

bool Parser::IsAtEnd() const {
	return m_Current >= m_Tokens.size();
}

Token Parser::Advance() {
	if (!IsAtEnd())
		++m_Current;
	return GetPrevious();
}

Token Parser::Consume(TokenType type, std::string_view msg) {
	if (MatchCurrent(type))
		return Advance();

	m_Context.RegisterTokenError(GetCurrent(), msg);
	throw std::logic_error(msg.data());
}

Token Parser::GetCurrent() const {
	if (IsAtEnd())
		return GetPrevious();
	return m_Tokens[m_Current];
}

Token Parser::GetPrevious() const {
	return m_Tokens[m_Current - 1];
}

}
