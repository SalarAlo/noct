#pragma once

#include <initializer_list>
#include <memory>
#include <set>
#include <vector>

#include "noct/Context.h"

#include "noct/lexer/Token.h"
#include "noct/lexer/TokenType.h"

#include "noct/parser/expression/ExpressionFwd.h"

#include "noct/parser/statement/Statement.h"

namespace Noct {

class Parser {
public:
	Parser(const std::vector<Token>& tokens, Context& ctx);
	StatementPtrVector Parse();

private:
	std::unique_ptr<Noct::Statement> Stmt();
	StatementPtr Decleration();

	StatementPtr PrintStmt();
	StatementPtr BlockStmt();
	StatementPtr ExpressionStmt();
	StatementPtr IfStmt();
	StatementPtr WhileStmt();
	StatementPtr ForStmt();
	StatementPtr BreakStmt();
	StatementPtr VariableDecl();
	StatementPtr FunctionDecl();

	ExpressionPtr Expr();
	ExpressionPtr Assignment();
	ExpressionPtr Or();
	ExpressionPtr And();
	ExpressionPtr Ternary();
	ExpressionPtr Equality();
	ExpressionPtr Comparison();
	ExpressionPtr Term();
	ExpressionPtr Factor();
	ExpressionPtr Unary();
	ExpressionPtr Call();
	ExpressionPtr IncDec();
	ExpressionPtr Primary();

	ExpressionPtr RecoverRhs(TokenType type);

	void Synchronize();

	std::vector<ExpressionPtr> GetArguments();
	std::vector<Token> GetParameters();

	bool MatchAny(const std::initializer_list<TokenType>& types);
	bool Check(TokenType type);
	bool MatchCurrent(TokenType type);
	bool IsAtEnd() const;

	Token Advance();
	Token Consume(TokenType type, std::string_view msg);
	Token GetCurrent() const;
	Token GetPrevious() const;

private:
	const std::vector<Token>& m_Tokens;
	static const std::set<TokenType> s_BinaryTokenTypes;
	Context& m_Context;
	std::size_t m_Current {};
};

}
