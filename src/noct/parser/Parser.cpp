#include "noct/parser/Parser.h"

#include <fmt/format.h>
#include <memory>
#include <set>
#include <string>
#include <initializer_list>

#include "noct/Helpers.h"
#include "noct/exceptions/RuntimeException.h"
#include "noct/lexer/Token.h"
#include "noct/lexer/TokenType.h"

#include "noct/parser/expression/MakeExpression.h"
#include "noct/parser/statement/MakeStatement.h"
#include "noct/parser/statement/StatementFwd.h"

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

std::vector<StatementPtr> Parser::Parse() {
	std::vector<StatementPtr> statements {};

	try {
		while (!IsAtEnd()) {
			statements.push_back(Decleration());
		}
		return statements;
	} catch (const RuntimeError& e) {
		m_Context.RegisterTokenError(e.GetToken(), e.what());
		return {};
	}
}

std::unique_ptr<Statement> Parser::Stmt() {
	if (MatchCurrent(TokenType::Print))
		return PrintStmt();

	if (MatchCurrent(TokenType::LeftBrace)) {
		return BlockStmt();
	}

	if (MatchCurrent(TokenType::If)) {
		return IfStmt();
	}

	if (MatchCurrent(TokenType::While)) {
		return WhileStmt();
	}

	if (MatchCurrent(TokenType::For)) {
		return ForStmt();
	}
	if (MatchCurrent(TokenType::Break)) {
		return BreakStmt();
	}

	return ExpressionStmt();
}

std::unique_ptr<Statement> Parser::Decleration() {
	try {
		if (MatchCurrent(TokenType::Var))
			return VariableDecl();

		return Stmt();
	} catch (const RuntimeError& e) {
		m_Context.RegisterTokenError(e.GetToken(), e.what());
		Synchronize();
		return nullptr;
	}
}

std::unique_ptr<Statement> Parser::PrintStmt() {
	auto value { Expr() };
	Consume(TokenType::Semicolon, "Semicolon (';') after print statemenet exprected");
	return make_statement<PrintStatement>(std::move(value));
}

std::unique_ptr<Statement> Parser::IfStmt() {
	Consume(TokenType::LeftParen, "Left Parenthesis '(' expected after if statement)");
	auto condition { Expr() };
	Consume(TokenType::RightParen, "Right Parenthesis ')' expected after if condition)");

	auto trueStmt { Stmt() };
	std::unique_ptr<Statement> falseStmt { nullptr };

	if (MatchCurrent(TokenType::Else)) {
		falseStmt = Stmt();
	}

	return make_statement<IfStatement>(
	    std::move(trueStmt),
	    std::move(condition),
	    std::move(falseStmt));
}

std::unique_ptr<Statement> Parser::WhileStmt() {
	Consume(TokenType::LeftParen, "Left Parenthesis '(' expected after while statement)");
	auto condition { Expr() };
	Consume(TokenType::RightParen, "Right Parenthesis ')' expected after while condition)");

	auto body = Stmt();

	return make_statement<WhileStatement>(
	    std::move(condition),
	    std::move(body));
}

std::unique_ptr<Statement> Parser::BreakStmt() {
	Consume(TokenType::Semicolon, "Exptected a ';' after a break statement");
	return make_statement<BreakStatement>();
}

std::unique_ptr<Statement> Parser::ForStmt() {
	Consume(TokenType::LeftParen, "Left Parenthesis '(' expected after while statement)");
	std::unique_ptr<Statement> initialiser { nullptr };

	if (MatchCurrent(TokenType::Semicolon)) {
	} else if (MatchCurrent(TokenType::Var)) {
		initialiser = VariableDecl();
	} else {
		initialiser = ExpressionStmt();
	}

	std::unique_ptr<Expression> condition { nullptr };
	if (!Check(TokenType::Semicolon)) {
		condition = Expr();
	}
	Consume(TokenType::Semicolon, "Expected ';' after loop condition");

	std::unique_ptr<Expression> increment { nullptr };
	if (!Check(RightParen)) {
		increment = Expr();
	}
	Consume(TokenType::RightParen, "Expected ')' after loop increment");

	auto body = Stmt();

	if (increment) {
		std::vector<std::unique_ptr<Statement>> guts {};
		guts.push_back(std::move(body));
		guts.push_back(make_statement<ExpressionStatement>(std::move(increment)));
		body = make_statement<BlockStatement>(std::move(guts));
	}
	if (!condition) {
		condition = std::make_unique<Expression>(Literal { true });
	}

	std::vector<StatementPtr> guts {};

	auto loop = make_statement<WhileStatement>(
	    std::move(condition),
	    std::move(body));

	if (initialiser) {
		guts.push_back(std::move(initialiser));
	}

	guts.push_back(std::move(loop));

	return make_statement<BlockStatement>(std::move(guts));
}

std::unique_ptr<Statement> Parser::ExpressionStmt() {
	auto value { Expr() };
	Consume(TokenType::Semicolon, "expected ';' after print statemenet");
	return make_statement<ExpressionStatement>(std::move(value));
}

std::unique_ptr<Statement> Parser::VariableDecl() {
	Token name = Consume(TokenType::Identifier, "Expect variable name.");

	std::unique_ptr<Expression> initializer = nullptr;
	if (MatchCurrent(TokenType::Equal)) {
		initializer = Expr();
	}

	Consume(Semicolon, "Expected ';' after variable decleration");
	return make_statement<VariableDecleration>(name, std::move(initializer));
}

std::unique_ptr<Statement> Parser::BlockStmt() {
	std::vector<std::unique_ptr<Statement>> statements {};
	while (GetCurrent().Type != TokenType::RightBrace && !IsAtEnd()) {
		statements.push_back(Decleration());
	}

	Consume(TokenType::RightBrace, "Expected '}' to end block");

	return make_statement<BlockStatement>(std::move(statements));
}

std::unique_ptr<Expression> Parser::Expr() {
	return Assignment();
}

std::unique_ptr<Expression> Parser::Assignment() {
	std::unique_ptr<Expression> expr { Or() };

	if (MatchCurrent(Equal)) {
		Token equalOperator { GetPrevious() };
		std::unique_ptr<Expression> lhs { Assignment() };

		if (auto var = std::get_if<Variable>(&expr->Value)) {
			return make_expression<Assign>(var->Name, std::move(lhs));
		}

		throw RuntimeError(equalOperator, "Invalid Assigment Target");
	}

	return expr;
}

std::unique_ptr<Expression> Parser::Or() {
	auto left { And() };
	while (MatchCurrent(TokenType::Or)) {
		auto orOp { GetPrevious() };
		auto right { And() };
		left = make_expression<Logical>(std::move(left), orOp, std::move(right));
	}

	return left;
}

std::unique_ptr<Expression> Parser::And() {
	auto left { Comma() };

	while (MatchCurrent(TokenType::And)) {
		auto andOp { GetPrevious() };
		auto right { Comma() };
		left = make_expression<Logical>(std::move(left), andOp, std::move(right));
	}

	return left;
}

std::unique_ptr<Expression> Parser::Comma() {
	auto left { Ternary() };

	while (MatchCurrent(TokenType::Comma)) {
		auto op { GetPrevious() };
		auto right { Ternary() };
		left = make_expression<Binary>(std::move(left), op, std::move(right));
	}

	return left;
}

std::unique_ptr<Expression> Parser::Ternary() {
	auto condition { Equality() };

	if (MatchCurrent(TokenType::QuestionMark)) {
		auto trueExpr { Expr() };
		Consume(TokenType::Colon, "Expcted ':' after '?'");
		auto falseExpr { Ternary() };
		condition = make_expression<Noct::Ternary>(std::move(condition), std::move(trueExpr), std::move(falseExpr));
	}

	return condition;
}

std::unique_ptr<Expression> Parser::Equality() {
	auto left { Comparison() };

	while (MatchAny({ BangEqual, EqualEqual })) {
		auto op { GetPrevious() };
		auto right { Comparison() };
		left = make_expression<Binary>(std::move(left), op, std::move(right));
	}

	return left;
}

std::unique_ptr<Expression> Parser::Comparison() {
	auto left { Term() };

	while (MatchAny({ Greater, GreaterEqual, Less, LessEqual })) {
		auto op { GetPrevious() };
		auto right { Term() };
		left = make_expression<Binary>(std::move(left), op, std::move(right));
	}

	return left;
}

std::unique_ptr<Expression> Parser::Term() {
	auto left { Factor() };

	while (MatchAny({ Plus, Minus })) {
		auto op { GetPrevious() };
		auto right { Factor() };
		left = make_expression<Binary>(std::move(left), op, std::move(right));
	}

	return left;
}

std::unique_ptr<Expression> Parser::Factor() {
	auto left { Unary() };

	while (MatchAny({ Star, Slash, Percentage })) {
		auto op { GetPrevious() };
		auto right { Unary() };
		left = make_expression<Binary>(std::move(left), op, std::move(right));
	}

	return left;
}

std::unique_ptr<Expression> Parser::Unary() {
	if (MatchAny({ Minus, Bang })) {
		auto op { GetPrevious() };
		auto right { Unary() };
		return make_expression<Noct::Unary>(op, std::move(right));
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
		return make_expression<Literal>(true);
	}

	if (MatchCurrent(False)) {
		return make_expression<Literal>(false);
	}

	if (MatchCurrent(Nil)) {
		return make_expression<Literal>(std::monostate {});
	}

	if (MatchAny({ String, Number })) {
		return make_expression<Literal>(GetPrevious().Literal);
	}

	if (MatchCurrent(LeftParen)) {
		auto guts { Expr() };
		Consume(RightParen, "Expected ')' after expression");
		return make_expression<Grouping>(std::move(guts));
	}

	if (MatchCurrent(Identifier)) {
		return make_expression<Variable>(GetPrevious());
	}

	throw RuntimeError(GetCurrent(), "Expected Expression");
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

bool Parser::Check(TokenType type) {
	return GetCurrent().Type == type;
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

	throw RuntimeError(GetCurrent(), msg.data());
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
