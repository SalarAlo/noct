#pragma once

#include <memory>
#include <set>
#include <vector>
#include <initializer_list>

#include "noct/Context.h"

#include "noct/lexer/Token.h"
#include "noct/lexer/TokenType.h"

#include "noct/parser/expression/Expression.h"
#include "noct/parser/expression/ExpressionDef.h"

#include "noct/parser/statement/Statement.h"

namespace Noct {

class Parser {
public:
	Parser(const std::vector<Token>& tokens, Context& ctx);
	std::vector<StatementPtr> Parse();

private:
	std::unique_ptr<Noct::Statement> Stmt();
	std::unique_ptr<Statement> Decleration();

	std::unique_ptr<Statement> PrintStmt();
	std::unique_ptr<Statement> BlockStmt();
	std::unique_ptr<Statement> ExpressionStmt();
	std::unique_ptr<Statement> IfStmt();
	std::unique_ptr<Statement> WhileStmt();
	std::unique_ptr<Statement> ForStmt();
	std::unique_ptr<Statement> BreakStmt();
	std::unique_ptr<Statement> VariableDecl();

	std::unique_ptr<Expression> Expr();
	std::unique_ptr<Expression> Assignment();
	std::unique_ptr<Expression> Or();
	std::unique_ptr<Expression> And();
	std::unique_ptr<Expression> Comma();
	std::unique_ptr<Expression> Ternary();
	std::unique_ptr<Expression> Equality();
	std::unique_ptr<Expression> Comparison();
	std::unique_ptr<Expression> Term();
	std::unique_ptr<Expression> Factor();
	std::unique_ptr<Expression> Unary();
	std::unique_ptr<Expression> Primary();

	std::unique_ptr<Expression> RecoverRhs(TokenType type);

	void Synchronize();

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
