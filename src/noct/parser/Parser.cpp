#include "noct/parser/Parser.h"

#include <memory>
#include <stdexcept>

#include "noct/lexer/TokenType.h"
#include "noct/parser/Binary.h"
#include "noct/parser/Grouping.h"
#include "noct/parser/Literal.h"
#include "noct/parser/Ternary.h"
#include "noct/parser/Unary.h"

using enum Noct::TokenType;

namespace Noct {

const std::set<TokenType> Parser::s_BinaryTokenTypes {
	TokenType::Comma,
	TokenType::BangEqual,
	TokenType::EqualEqual,
	TokenType::Greater,
	TokenType::GreaterEqual,
	TokenType::Less,
	TokenType::LessEqual,
	TokenType::Plus,
	TokenType::Star,
	TokenType::Slash,
};

Parser::Parser(const std::vector<Token>& tokens, Context& ctx)
    : m_Tokens(tokens)
    , m_Context(ctx) { };

void Parser::Synchronize() {
	Advance();

	while (!IsAtEnd()) {
		if (GetPrevious().Type == Semicolon)
			return;

		switch (GetCurrent().Type) {
		case Class:
		case Fun:
		case Var:
		case For:
		case If:
		case While:
		case Print:
		case Return:
			return;
		default:
			Advance();
		}
	}
}

std::unique_ptr<Expression> Parser::Parse() {
	try {
		auto out { Expr() };
		if (!MatchCurrent(Eof)) {
			m_Context.RegisterSourceCodeError(0, "Unexpected junk after expression");
		}
		return out;
	} catch (const std::exception& e) {
		return nullptr;
	}
}

std::unique_ptr<Expression> Parser::Expr() {
	return Comma();
}

std::unique_ptr<Expression> Parser::Comma() {
	auto left { Ternary() };

	while (MatchCurrent(TokenType::Comma)) {
		auto commaOperator { GetPrevious() };
		auto right { Ternary() };
		left = std::make_unique<Binary>(std::move(left), commaOperator, std::move(right));
	}

	return left;
}

std::unique_ptr<Expression> Parser::Ternary() {
	auto condition { Equality() };

	if (MatchCurrent(TokenType::QuestionMark)) {
		auto trueExpr { Expr() };
		Consume(TokenType::Colon, "Expcted ':' after '?'");
		auto falseExpr { Ternary() };
		condition = std::make_unique<Noct::Ternary>(std::move(condition), std::move(trueExpr), std::move(falseExpr));
	}

	return condition;
}

std::unique_ptr<Expression> Parser::Equality() {
	auto left { Comparison() };

	while (MatchAny({ BangEqual, EqualEqual })) {
		auto op { GetPrevious() };
		auto right { Comparison() };
		left = std::make_unique<Binary>(std::move(left), op, std::move(right));
	}

	return left;
}

std::unique_ptr<Expression> Parser::Comparison() {
	auto left { Term() };

	while (MatchAny({ Greater, GreaterEqual, Less, LessEqual })) {
		auto op { GetPrevious() };
		auto right { Term() };
		left = std::make_unique<Binary>(std::move(left), op, std::move(right));
	}

	return left;
}

std::unique_ptr<Expression> Parser::Term() {
	auto left { Factor() };

	while (MatchAny({ Plus, Minus })) {
		auto op { GetPrevious() };
		auto right { Factor() };
		left = std::make_unique<Binary>(std::move(left), op, std::move(right));
	}

	return left;
}

std::unique_ptr<Expression> Parser::Factor() {
	auto left { Unary() };

	while (MatchAny({ Star, Slash })) {
		auto op { GetPrevious() };
		auto right { Unary() };
		left = std::make_unique<Binary>(std::move(left), op, std::move(right));
	}

	return left;
}

std::unique_ptr<Expression> Parser::Unary() {
	if (MatchAny({ Minus, Bang })) {
		auto op { GetPrevious() };
		auto right { Unary() };
		return std::make_unique<Noct::Unary>(op, std::move(right));
	}

	return Primary();
}

std::unique_ptr<Expression> Parser::Primary() {
	if (s_BinaryTokenTypes.contains(GetCurrent().Type)) {
		const Token falsyOperator { Advance() };
		// TODO: Register Error Properly
		return nullptr;
	}

	if (MatchCurrent(True)) {
		return std::make_unique<Literal>(true);
	}

	if (MatchCurrent(False)) {
		return std::make_unique<Literal>(false);
	}

	if (MatchCurrent(Nil)) {
		return std::make_unique<Literal>(std::monostate {});
	}

	if (MatchAny({ String, Number })) {
		return std::make_unique<Literal>(GetPrevious().Literal);
	}

	if (MatchCurrent(LeftParen)) {
		auto guts { Expr() };
		Consume(RightParen, "Expected ')' after expression");
		return std::make_unique<Grouping>(std::move(guts));
	}

	throw std::logic_error("Expected Expression");
}

bool Parser::MatchAny(const std::initializer_list<TokenType>& types) {
	auto current { GetCurrent() };

	for (const auto& t : types) {
		if (t == current.Type) {
			Advance();
			return true;
		}
	}

	return false;
}

bool Parser::MatchCurrent(TokenType type) {
	if (GetCurrent().Type == type) {
		Advance();
		return true;
	}
	return false;
}

bool Parser::IsAtEnd() const {
	return m_Current >= m_Tokens.size();
}

Token Parser::Advance() {
	if (IsAtEnd()) {
		return GetPrevious();
	}

	return m_Tokens[m_Current++];
}

Token Parser::Consume(TokenType type, std::string_view msg) {
	if (MatchCurrent(type)) {
		return GetPrevious();
	}

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
