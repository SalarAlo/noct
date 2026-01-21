#include "noct/parser/Parser.h"

#include <memory>
#include <stdexcept>

#include "noct/Helpers.h"
#include "noct/exceptions/RuntimeException.h"
#include "noct/lexer/TokenType.h"
#include "noct/parser/statement/ExpressionStatement.h"
#include "noct/parser/statement/PrintStatement.h"
#include "noct/parser/statement/Statement.h"
#include "noct/parser/statement/VariableDecleration.h"

#include "noct/parser/expression/Binary.h"
#include "noct/parser/expression/Grouping.h"
#include "noct/parser/expression/Literal.h"
#include "noct/parser/expression/Ternary.h"
#include "noct/parser/expression/Unary.h"
#include "noct/parser/expression/Assign.h"
#include "noct/parser/expression/Variable.h"

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

std::vector<std::unique_ptr<Statement>> Parser::Parse() {
	std::vector<std::unique_ptr<Noct::Statement>> statements {};

	try {
		while (!IsAtEnd()) {
			statements.push_back(Decleration());
		}
		return statements;
	} catch (const std::exception& e) {
		m_Context.RegisterSourceCodeError(0, e.what());
		return {};
	}
}

std::unique_ptr<Statement> Parser::Stmt() {
	if (MatchCurrent(TokenType::Print))
		return PrintStmt();

	return ExpressionStmt();
}

std::unique_ptr<Statement> Parser::Decleration() {
	try {
		if (MatchCurrent(TokenType::Var))
			return VariableDecl();

		return Stmt();
	} catch (const RuntimeError& e) {
		Synchronize();
		return nullptr;
	}
}

std::unique_ptr<PrintStatement> Parser::PrintStmt() {
	auto value { Expr() };
	Consume(TokenType::Semicolon, "Semicolon (';') after print statemenet exprected");
	return std::make_unique<PrintStatement>(std::move(value));
}

std::unique_ptr<ExpressionStatement> Parser::ExpressionStmt() {
	auto value { Expr() };
	Consume(TokenType::Semicolon, "Semicolon (';') after print statemenet exprected");
	return std::make_unique<ExpressionStatement>(std::move(value));
}

std::unique_ptr<VariableDecleration> Parser::VariableDecl() {
	Token name = Consume(TokenType::Identifier, "Expect variable name.");

	std::unique_ptr<Expression> initializer = nullptr;
	if (MatchCurrent(TokenType::Equal)) {
		initializer = Expr();
	}

	Consume(Semicolon, "Expected ';' after variable decleration");
	return std::make_unique<VariableDecleration>(name, std::move(initializer));
}

std::unique_ptr<Expression> Parser::Expr() {
	return Assignment();
}

std::unique_ptr<Expression> Parser::Assignment() {
	std::unique_ptr<Expression> expr { Comma() };

	if (MatchCurrent(Equal)) {
		Token equalOperator { GetPrevious() };
		std::unique_ptr<Expression> lhs { Assignment() };

		if (auto x = dynamic_cast<Variable*>(expr.get())) {
			return std::make_unique<Assign>(x->Name, std::move(lhs));
		}

		throw std::logic_error("Invalid Assigment Target");
	}

	return expr;
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

		std::string errorMsg { "Missing left hand operand before binary operator " };
		errorMsg.append(ToString(falsyOperator.Type));

		m_Context.RegisterSourceCodeError(falsyOperator.Line, errorMsg);
		return RecoverRhs(falsyOperator.Type);
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

	if (MatchCurrent(Identifier)) {
		return std::make_unique<Variable>(GetPrevious());
	}

	throw std::logic_error("Expected Expression");
}

std::unique_ptr<Expression> Parser::RecoverRhs(TokenType type) {
	switch (type) {
	case TokenType::Comma:
		return Ternary();

	case TokenType::BangEqual:
	case TokenType::EqualEqual:
		return Comparison();

	case TokenType::Greater:
	case TokenType::GreaterEqual:
	case TokenType::Less:
	case TokenType::LessEqual:
		return Term();

	case TokenType::Plus:
	case TokenType::Minus:
		return Factor();

	case TokenType::Star:
	case TokenType::Slash:
		return Unary();

	default:
		return Expr();
	}
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
	return m_Current < m_Tokens.size() ? m_Tokens[m_Current].Type == TokenType::Eof : false;
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
