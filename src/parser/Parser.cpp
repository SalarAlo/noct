#include <fmt/format.h>

#include <initializer_list>
#include <memory>
#include <set>
#include <string>

#include "parser/Parser.h"

#include "Helpers.h"

#include "exceptions/RuntimeException.h"

#include "lexer/Token.h"
#include "lexer/TokenType.h"

#include "parser/expression/Expression.h"
#include "parser/expression/ExpressionFwd.h"
#include "parser/expression/LiteralNummifier.h"
#include "parser/expression/MakeExpression.h"

#include "parser/expression/variants/Get.h"
#include "parser/expression/variants/This.h"

#include "parser/statement/BlockStatement.h"
#include "parser/statement/ClassDecleration.h"
#include "parser/statement/FunctionDecleration.h"
#include "parser/statement/ImportStatement.h"
#include "parser/statement/MakeStatement.h"
#include "parser/statement/StatementFwd.h"

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
	TokenType::PlusEqual,
	TokenType::MinusEqual,
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

StatementPtrVector Parser::parse() {
	StatementPtrVector statements {};

	try {
		while (!IsAtEnd()) {
			if (auto stmt = decleration()) {
				statements.push_back(std::move(stmt));
			}
		}

		return statements;
	} catch (const RuntimeError& e) {
		m_Context.ReportParseError(e.Where(), e.what());
		return {};
	}
}

StatementPtr Parser::decleration() {
	try {
		if (MatchCurrent(TokenType::Var))
			return variable_decl();

		if (MatchCurrent(TokenType::Import))
			return import_stmt();

		if (Check(TokenType::Fn)) {
			if (auto res = function_decl(); res)
				return res;
			else
				return expression_stmt();
		}

		if (MatchCurrent(TokenType::Class)) {
			return class_decl();
		}

		return stmt();
	} catch (const RuntimeError& e) {
		m_Context.ReportParseError(e.Where(), e.what());
		Synchronize();
		return nullptr;
	}
}

StatementPtr Parser::stmt() {
	if (MatchCurrent(TokenType::Print))
		return print_stmt();

	if (MatchCurrent(TokenType::LeftBrace)) {
		return block_stmt();
	}

	if (MatchCurrent(TokenType::If)) {
		return if_stmt();
	}

	if (MatchCurrent(TokenType::Return)) {
		return return_stmt();
	}

	if (MatchCurrent(TokenType::While)) {
		return while_stmt();
	}

	if (MatchCurrent(TokenType::For)) {
		return for_stmt();
	}
	if (MatchCurrent(TokenType::Break)) {
		return break_stmt();
	}

	return expression_stmt();
}

StatementPtr Parser::return_stmt() {
	auto retToken { GetPrevious() };
	if (MatchCurrent(TokenType::Semicolon)) {
		return make_statement<ReturnStatement>(retToken, nullptr);
	}

	auto value { expr() };
	Consume(TokenType::Semicolon, "';' after return statemenet exprected");
	return make_statement<ReturnStatement>(retToken, std::move(value));
}

StatementPtr Parser::import_stmt() {
	Token name { Consume(TokenType::Identifier, "Expected module name after import.") };

	if (MatchCurrent(TokenType::Dot)) {
		throw RuntimeError(GetPrevious(), "Import only supports a plain module identifier.");
	}

	MatchCurrent(TokenType::Semicolon);
	return make_statement<ImportStatement>(name);
}

StatementPtr Parser::print_stmt() {
	auto value { expr() };
	Consume(TokenType::Semicolon, "Semicolon (';') after print statemenet exprected");
	return make_statement<PrintStatement>(std::move(value));
}

StatementPtr Parser::if_stmt() {
	auto condition { expr() };

	auto true_stmt { stmt() };
	StatementPtr false_stmt { nullptr };

	if (MatchCurrent(TokenType::Else)) {
		false_stmt = stmt();
	}

	return make_statement<IfStatement>(
	    std::move(true_stmt),
	    std::move(condition),
	    std::move(false_stmt));
}

StatementPtr Parser::while_stmt() {
	auto condition { expr() };

	auto body = stmt();

	return make_statement<WhileStatement>(
	    std::move(condition),
	    std::move(body));
}

StatementPtr Parser::break_stmt() {
	auto b { GetPrevious() };
	Consume(TokenType::Semicolon, "Exptected a ';' after a break statement");
	return make_statement<BreakStatement>(b);
}

StatementPtr Parser::for_stmt() {
	using enum TokenType;

	Consume(TokenType::LeftParen, "Missing '(' expected after while statement)");
	StatementPtr initialiser { nullptr };

	if (MatchCurrent(TokenType::Semicolon)) {
	} else if (MatchCurrent(TokenType::Var)) {
		initialiser = variable_decl();
	} else {
		initialiser = expression_stmt();
	}

	ExpressionPtr condition { nullptr };
	if (!Check(TokenType::Semicolon)) {
		condition = expr();
	}
	Consume(TokenType::Semicolon, "Missing ';' after loop condition");

	ExpressionPtr increment { nullptr };
	if (!Check(RightParen)) {
		increment = expr();
	}
	Consume(TokenType::RightParen, "Missing ')' after loop increment");

	auto body = stmt();

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

StatementPtr Parser::expression_stmt() {
	auto value { expr() };
	Consume(TokenType::Semicolon, "expected ';' after expression statemenet");
	return make_statement<ExpressionStatement>(std::move(value));
}

StatementPtr Parser::variable_decl() {
	using enum TokenType;

	Token name = Consume(Identifier, "Expect variable name.");

	ExpressionPtr initializer = nullptr;
	if (MatchCurrent(TokenType::Equal)) {
		initializer = expr();
	}

	Consume(Semicolon, "Expected ';' after variable decleration");
	return make_statement<VariableDecleration>(name, std::move(initializer));
}

StatementPtr Parser::function_decl() {
	if (!CheckNext(TokenType::Identifier)) {
		return nullptr;
	}

	Consume(TokenType::Fn, "Expected function to start with fn keyword.");

	Token name = Consume(TokenType::Identifier, "Expected variable name.");

	Consume(TokenType::LeftParen, "Expected '(' after function name.");
	auto args { GetParameters() };
	Consume(TokenType::RightParen, "Expected ')' after argument list.");
	Consume(TokenType::LeftBrace, "Expected '{' after function decleration list.");

	BlockStatement body { std::get<BlockStatement>(std::move(block_stmt()->Instruction)) };
	MatchCurrent(TokenType::Semicolon);

	return make_statement<FunctionDecleration>(name, args, std::move(body.Statements));
}

StatementPtr Parser::class_decl() {
	Token name { Consume(TokenType::Identifier, "Expected class name.") };

	std::vector<FunctionDecleration> methods {};

	Consume(TokenType::LeftBrace, "Expected '{' after class name.");

	while (Check(TokenType::Fn)) {
		if (auto fnDeclStmt { function_decl() }; fnDeclStmt) {
			auto fnDecl { std::move(std::get<FunctionDecleration>(fnDeclStmt->Instruction)) };
			methods.push_back(std::move(fnDecl));
			continue;
		}

		throw RuntimeError(GetCurrent(), "Method Decleration failed");
	}

	Consume(TokenType::RightBrace, "Expected '}' after class definition.");

	return make_statement<ClassDecleration>(name, std::move(methods));
}

StatementPtr Parser::block_stmt() {
	StatementPtrVector statements {};

	while (GetCurrent().Type != TokenType::RightBrace && !IsAtEnd()) {
		statements.push_back(decleration());
	}

	Consume(TokenType::RightBrace, "Expected '}' to end block");

	return make_statement<BlockStatement>(std::move(statements));
}

ExpressionPtr Parser::expr() {
	return assignment();
}

ExpressionPtr Parser::assignment() {
	ExpressionPtr expr { Or() };

	if (MatchAny({ TokenType::Equal, TokenType::PlusEqual, TokenType::MinusEqual })) {
		Token equalOperator { GetPrevious() };
		ExpressionPtr value { assignment() };

		if (auto var { std::get_if<Variable>(&expr->Value) }) {
			return make_expression<Assign>(var->Name, equalOperator, std::move(value));
		} else if (auto get { std::get_if<Get>(&expr->Value) }) {
			return make_expression<Set>(get->Name, equalOperator, std::move(get->Instance), std::move(value));
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
	auto condition { equality() };

	if (MatchCurrent(TokenType::QuestionMark)) {
		auto trueExpr { expr() };
		Consume(TokenType::Colon, "Expcted ':' after '?'");
		auto falseExpr { Ternary() };
		condition = make_expression<Noct::Ternary>(std::move(condition), std::move(trueExpr), std::move(falseExpr));
	}

	return condition;
}

ExpressionPtr Parser::equality() {
	using enum TokenType;
	auto left { comparison() };

	while (MatchAny({ BangEqual, EqualEqual })) {
		auto op { GetPrevious() };
		auto right { comparison() };
		left = make_expression<Binary>(std::move(left), op, std::move(right));
	}

	return left;
}

ExpressionPtr Parser::comparison() {
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
	auto expr { IncDec() };

	while (true) {
		if (MatchCurrent(LeftParen)) {
			auto paren { GetPrevious() };
			auto args = GetCommaSeperatedExpressions();
			Consume(RightParen, "Expected ')' after arguments.");
			expr = make_expression<Noct::Call>(std::move(expr), std::move(args), paren);
		} else if (MatchCurrent(Dot)) {
			Token name { Consume(Identifier, "Expected a property name after '.'.") };
			expr = make_expression<Noct::Get>(std::move(expr), name);
		} else {
			break;
		}
	}

	return expr;
}

ExpressionPtr Parser::IncDec() {
	using enum TokenType;
	auto target { Primary() };

	while (MatchAny({ PlusPlus, MinusMinus })) {
		target = make_expression<Noct::Unary>(GetPrevious(), std::move(target));
	}

	return target;
}

std::vector<ExpressionPtr> Parser::GetCommaSeperatedExpressions() {
	using enum TokenType;
	std::vector<ExpressionPtr> arguments {};

	if (Check(RightParen)) {
		return arguments;
	}

	do {
		arguments.emplace_back(expr());
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

		m_Context.ReportParseError(falsyOperator.Line, errorMsg);
		return RecoverRhs(falsyOperator.Type);
	}

	if (MatchCurrent(TokenType::Fn)) {
		Consume(TokenType::LeftParen, "Expected '(' after lambda fn");
		auto params { GetParameters() };
		Consume(TokenType::RightParen, "Expected ')' after lambda arguments.");
		Consume(TokenType::LeftBrace, "Expected '{' after lambda argument list.");

		BlockStatement body { std::get<BlockStatement>(std::move(block_stmt()->Instruction)) };

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
		return make_expression<Literal>(std::monostate {});
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
		auto guts { expr() };
		Consume(TokenType::RightParen, "Expected ')' after expression");
		return make_expression<Grouping>(std::move(guts));
	}

	if (MatchCurrent(TokenType::Identifier)) {
		return make_expression<Variable>(GetPrevious());
	}

	if (MatchCurrent(TokenType::This)) {
		return make_expression<This>(GetPrevious());
	}

	throw RuntimeError(GetCurrent(), "Expected Expression");
}

ExpressionPtr Parser::RecoverRhs(TokenType type) {
	switch (type) {
	case TokenType::BangEqual:
	case TokenType::EqualEqual:
		return comparison();

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
		return expr();
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
	if (!IsAtEnd()) {
		m_Current++;
	}

	return m_Tokens[m_Current];
}

Token Parser::Consume(TokenType type, std::string_view msg) {
	if (MatchCurrent(type)) {
		return GetPrevious();
	}

	throw RuntimeError(GetCurrent(), msg.data());
}

Token Parser::GetCurrent() const {
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
