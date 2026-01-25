#include <fmt/format.h>

#include <initializer_list>
#include <memory>
#include <set>
#include <string>

#include "noct/parser/Parser.h"

#include "noct/Helpers.h"

#include "noct/exceptions/RuntimeException.h"

#include "noct/lexer/Token.h"
#include "noct/lexer/TokenType.h"

#include "noct/parser/expression/Expression.h"
#include "noct/parser/expression/LiteralNummifier.h"
#include "noct/parser/expression/MakeExpression.h"

#include "noct/parser/statement/BlockStatement.h"
#include "noct/parser/statement/FunctionDecleration.h"
#include "noct/parser/statement/MakeStatement.h"
#include "noct/parser/statement/StatementFwd.h"

namespace Noct {

const std::set<TokenType> Parser::s_BinaryTokenTypes {
	TokenType::BangEqual,
	TokenType::EqualEqual,
	TokenType::Percentage,
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
	using enum TokenType;
	Advance();

	while (!IsAtEnd()) {
		if (GetPrevious().Type == Semicolon)
			return;

		switch (GetCurrent().Type) {
		case Class:
		case Fn:
		case Make:
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

StatementPtrVector Parser::Parse() {
	StatementPtrVector statements {};

	try {
		while (!IsAtEnd()) {
			if (auto stmt = Decleration()) {
				statements.push_back(std::move(stmt));
			}
		}

		return statements;
	} catch (const RuntimeError& e) {
		m_Context.RegisterTokenError(e.GetToken(), e.what());
		return {};
	}
}

StatementPtr Parser::Decleration() {
	try {
		if (MatchCurrent(TokenType::Make))
			return VariableDecl();

		if (Check(TokenType::Fn)) {
			if (auto res = FunctionDecl(); res)
				return res;
			else
				return ExpressionStmt();
		}

		return Stmt();
	} catch (const RuntimeError& e) {
		m_Context.RegisterTokenError(e.GetToken(), e.what());
		Synchronize();
		return nullptr;
	}
}

StatementPtr Parser::Stmt() {
	if (MatchCurrent(TokenType::Print))
		return PrintStmt();

	if (MatchCurrent(TokenType::LeftBrace)) {
		return BlockStmt();
	}

	if (MatchCurrent(TokenType::If)) {
		return IfStmt();
	}

	if (MatchCurrent(TokenType::Return)) {
		return ReturnStmt();
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

StatementPtr Parser::ReturnStmt() {
	if (MatchCurrent(TokenType::Semicolon)) {
		return make_statement<ReturnStatement>(nullptr);
	}

	auto value { Expr() };
	Consume(TokenType::Semicolon, "';' after return statemenet exprected");
	return make_statement<ReturnStatement>(std::move(value));
}

StatementPtr Parser::PrintStmt() {
	auto value { Expr() };
	Consume(TokenType::Semicolon, "Semicolon (';') after print statemenet exprected");
	return make_statement<PrintStatement>(std::move(value));
}

StatementPtr Parser::IfStmt() {
	auto condition { Expr() };

	auto trueStmt { Stmt() };
	StatementPtr falseStmt { nullptr };

	if (MatchCurrent(TokenType::Else)) {
		falseStmt = Stmt();
	}

	return make_statement<IfStatement>(
	    std::move(trueStmt),
	    std::move(condition),
	    std::move(falseStmt));
}

StatementPtr Parser::WhileStmt() {
	auto condition { Expr() };

	auto body = Stmt();

	return make_statement<WhileStatement>(
	    std::move(condition),
	    std::move(body));
}

StatementPtr Parser::BreakStmt() {
	Consume(TokenType::Semicolon, "Exptected a ';' after a break statement");
	return make_statement<BreakStatement>();
}

StatementPtr Parser::ForStmt() {
	using enum TokenType;

	Consume(TokenType::LeftParen, "Left Parenthesis '(' expected after while statement)");
	StatementPtr initialiser { nullptr };

	if (MatchCurrent(TokenType::Semicolon)) {
	} else if (MatchCurrent(TokenType::Make)) {
		initialiser = VariableDecl();
	} else {
		initialiser = ExpressionStmt();
	}

	ExpressionPtr condition { nullptr };
	if (!Check(TokenType::Semicolon)) {
		condition = Expr();
	}
	Consume(TokenType::Semicolon, "Expected ';' after loop condition");

	ExpressionPtr increment { nullptr };
	if (!Check(RightParen)) {
		increment = Expr();
	}
	Consume(TokenType::RightParen, "Expected ')' after loop increment");

	auto body = Stmt();

	if (increment) {
		StatementPtrVector guts {};
		guts.push_back(std::move(body));
		guts.push_back(make_statement<ExpressionStatement>(std::move(increment)));
		body = make_statement<BlockStatement>(std::move(guts));
	}
	if (!condition) {
		condition = std::make_unique<Expression>(Literal { true });
	}

	StatementPtrVector guts {};

	auto loop = make_statement<WhileStatement>(
	    std::move(condition),
	    std::move(body));

	if (initialiser) {
		guts.push_back(std::move(initialiser));
	}

	guts.push_back(std::move(loop));

	return make_statement<BlockStatement>(std::move(guts));
}

StatementPtr Parser::ExpressionStmt() {
	auto value { Expr() };
	Consume(TokenType::Semicolon, "expected ';' after print statemenet");
	return make_statement<ExpressionStatement>(std::move(value));
}

StatementPtr Parser::VariableDecl() {
	using enum TokenType;

	Token name = Consume(Identifier, "Expect variable name.");

	ExpressionPtr initializer = nullptr;
	if (MatchCurrent(TokenType::Equal)) {
		initializer = Expr();
	}

	Consume(Semicolon, "Expected ';' after variable decleration");
	return make_statement<VariableDecleration>(name, std::move(initializer));
}

StatementPtr Parser::FunctionDecl() {
	if (!CheckNext(TokenType::Identifier)) {
		return nullptr;
	}

	Advance();
	Token name = Consume(TokenType::Identifier, "Expected variable name.");

	Consume(TokenType::LeftParen, "Expected '(' after function name.");
	auto args { GetParameters() };
	Consume(TokenType::RightParen, "Expected ')' after argument list.");
	Consume(TokenType::LeftBrace, "Expected '{' after function decleration list.");

	BlockStatement body { std::get<BlockStatement>(std::move(BlockStmt()->Instruction)) };

	return make_statement<FunctionDecleration>(name, args, std::move(body.Statements));
}

StatementPtr Parser::BlockStmt() {
	StatementPtrVector statements {};
	while (GetCurrent().Type != TokenType::RightBrace && !IsAtEnd()) {
		statements.push_back(Decleration());
	}

	Consume(TokenType::RightBrace, "Expected '}' to end block");

	return make_statement<BlockStatement>(std::move(statements));
}

ExpressionPtr Parser::Expr() {
	return Assignment();
}

ExpressionPtr Parser::Assignment() {
	std::unique_ptr expr { Or() };

	if (MatchCurrent(TokenType::Equal)) {
		Token equalOperator { GetPrevious() };
		std::unique_ptr value { Assignment() };

		if (auto var = std::get_if<Variable>(&expr->Value)) {
			return make_expression<Assign>(var->Name, std::move(value));
		}

		throw RuntimeError(equalOperator, "Invalid Assigment Target");
	}

	return expr;
}

ExpressionPtr Parser::Or() {
	auto left { And() };

	while (MatchCurrent(TokenType::Or)) {
		auto orOp { GetPrevious() };
		auto right { And() };
		left = make_expression<Logical>(std::move(left), orOp, std::move(right));
	}

	return left;
}

ExpressionPtr Parser::And() {
	auto left { Ternary() };

	while (MatchCurrent(TokenType::And)) {
		auto andOp { GetPrevious() };
		auto right { Ternary() };
		left = make_expression<Logical>(std::move(left), andOp, std::move(right));
	}

	return left;
}

ExpressionPtr Parser::Ternary() {
	auto condition { Equality() };

	if (MatchCurrent(TokenType::QuestionMark)) {
		auto trueExpr { Expr() };
		Consume(TokenType::Colon, "Expcted ':' after '?'");
		auto falseExpr { Ternary() };
		condition = make_expression<Noct::Ternary>(std::move(condition), std::move(trueExpr), std::move(falseExpr));
	}

	return condition;
}

ExpressionPtr Parser::Equality() {
	using enum TokenType;
	auto left { Comparison() };

	while (MatchAny({ BangEqual, EqualEqual })) {
		auto op { GetPrevious() };
		auto right { Comparison() };
		left = make_expression<Binary>(std::move(left), op, std::move(right));
	}

	return left;
}

ExpressionPtr Parser::Comparison() {
	using enum TokenType;
	auto left { Term() };

	while (MatchAny({ Greater, GreaterEqual, Less, LessEqual })) {
		auto op { GetPrevious() };
		auto right { Term() };
		left = make_expression<Binary>(std::move(left), op, std::move(right));
	}

	return left;
}

ExpressionPtr Parser::Term() {
	using enum TokenType;
	auto left { Factor() };

	while (MatchAny({ Plus, Minus })) {
		auto op { GetPrevious() };
		auto right { Factor() };
		left = make_expression<Binary>(std::move(left), op, std::move(right));
	}

	return left;
}

ExpressionPtr Parser::Factor() {
	using enum TokenType;
	auto left { Unary() };

	while (MatchAny({ Star, Slash, Percentage })) {
		auto op { GetPrevious() };
		auto right { Unary() };
		left = make_expression<Binary>(std::move(left), op, std::move(right));
	}

	return left;
}

ExpressionPtr Parser::Unary() {
	using enum TokenType;
	if (MatchAny({ Minus, Bang })) {
		auto op { GetPrevious() };
		auto right { Call() };
		return make_expression<Noct::Unary>(op, std::move(right));
	}

	return Call();
}

ExpressionPtr Parser::Call() {
	using enum TokenType;
	auto callee { IncDec() };

	while (MatchCurrent(LeftParen)) {
		auto args = GetArguments();
		Consume(RightParen, "Expected ')' after arguments.");
		return make_expression<Noct::Call>(std::move(callee), std::move(args));
	}

	return callee;
}

ExpressionPtr Parser::IncDec() {
	using enum TokenType;
	auto target { Primary() };

	while (MatchAny({ PlusPlus, MinusMinus })) {
		return make_expression<Noct::Unary>(GetPrevious(), std::move(target));
	}

	return target;
}

std::vector<ExpressionPtr> Parser::GetArguments() {
	using enum TokenType;
	std::vector<ExpressionPtr> arguments {};

	if (Check(RightParen)) {
		return arguments;
	}

	do {
		arguments.emplace_back(Expr());
	} while (MatchCurrent(Comma));

	return arguments;
}

std::vector<Token> Parser::GetParameters() {
	using enum TokenType;
	std::vector<Token> arguments {};

	if (!Check(Identifier)) {
		return arguments;
	}

	do {
		if (arguments.size() == 255) {
			throw RuntimeError(GetCurrent(), "Can not have more than 255 parameters.");
		}
		arguments.emplace_back(Consume(Identifier, "Expected identifier in parameter list"));
	} while (MatchCurrent(Comma));

	return arguments;
}

ExpressionPtr Parser::Primary() {
	if (s_BinaryTokenTypes.contains(GetCurrent().Type)) {
		const Token falsyOperator { Advance() };

		std::string errorMsg { "Missing left hand operand before binary operator " };
		errorMsg.append(ToString(falsyOperator.Type));

		m_Context.RegisterSourceCodeError(falsyOperator.Line, errorMsg);
		return RecoverRhs(falsyOperator.Type);
	}

	if (MatchCurrent(TokenType::Fn)) {
		Consume(TokenType::LeftParen, "Expected '(' after lambda fn");
		auto params { GetParameters() };
		Consume(TokenType::RightParen, "Expected ')' after lambda arguments.");
		Consume(TokenType::LeftBrace, "Expected '{' after lambda argument list.");

		BlockStatement body { std::get<BlockStatement>(std::move(BlockStmt()->Instruction)) };

		return make_expression<Lambda>(params, std::move(body.Statements));
	}

	if (MatchCurrent(TokenType::True)) {
		return make_expression<Literal>(true);
	}

	if (MatchCurrent(TokenType::Maybe)) {
		return make_expression<Maybe>();
	}

	if (MatchCurrent(TokenType::False)) {
		return make_expression<Literal>(false);
	}

	if (MatchCurrent(TokenType::Nil)) {
		return make_expression<Noct::Maybe>();
	}

	if (MatchCurrent(TokenType::Number)) {
		auto num { LiteralNumifier {}(GetPrevious().Lexeme) };
		return make_expression<Literal>(*num);
	}

	if (MatchCurrent(TokenType::String)) {
		auto strToken { GetPrevious() };
		auto str { strToken.Lexeme.substr(1, strToken.Lexeme.size() - 2) };
		return make_expression<Literal>(str);
	}

	if (MatchCurrent(TokenType::LeftParen)) {
		auto guts { Expr() };
		Consume(TokenType::RightParen, "Expected ')' after expression");
		return make_expression<Grouping>(std::move(guts));
	}

	if (MatchCurrent(TokenType::Identifier)) {
		return make_expression<Variable>(GetPrevious());
	}

	throw RuntimeError(GetCurrent(), "Expected Expression");
}

ExpressionPtr Parser::RecoverRhs(TokenType type) {
	switch (type) {
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

bool Parser::CheckNext(TokenType type) {
	return GetNext().Type == type;
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

Token Parser::GetNext() const {
	if (m_Current >= m_Tokens.size() - 1)
		return GetPrevious();

	return m_Tokens[m_Current + 1];
}

Token Parser::GetPrevious() const {
	return m_Tokens[m_Current - 1];
}

}
