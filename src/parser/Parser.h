#pragma once

#include <initializer_list>
#include <memory>
#include <set>
#include <vector>

#include "Context.h"

#include "lexer/Token.h"
#include "lexer/TokenType.h"

#include "parser/expression/ExpressionFwd.h"

#include "parser/statement/Statement.h"

namespace Noct {

class Parser {
public:
	Parser(const std::vector<Token>& tokens, Context& ctx);
	StatementPtrVector parse();

private:
	std::unique_ptr<Noct::Statement> stmt();
	StatementPtr decleration();

	StatementPtr print_stmt();
	StatementPtr block_stmt();
	StatementPtr expression_stmt();
	StatementPtr if_stmt();
	StatementPtr while_stmt();
	StatementPtr for_stmt();
	StatementPtr break_stmt();
	StatementPtr return_stmt();
	StatementPtr import_stmt();
	StatementPtr variable_decl();
	StatementPtr function_decl();
	StatementPtr class_decl();

	ExpressionPtr expr();
	ExpressionPtr assignment();
	ExpressionPtr Or();
	ExpressionPtr And();
	ExpressionPtr Ternary();
	ExpressionPtr equality();
	ExpressionPtr comparison();
	ExpressionPtr Term();
	ExpressionPtr Factor();
	ExpressionPtr Unary();
	ExpressionPtr Call();
	ExpressionPtr IncDec();
	ExpressionPtr Primary();

	ExpressionPtr RecoverRhs(TokenType type);

	void Synchronize();

	std::vector<ExpressionPtr> GetCommaSeperatedExpressions();
	std::vector<Token> GetParameters();

	bool MatchAny(const std::initializer_list<TokenType>& types);
	bool Check(TokenType type);
	bool CheckNext(TokenType type);
	bool MatchCurrent(TokenType type);
	bool IsAtEnd() const;

	Token Advance();
	Token Consume(TokenType type, std::string_view msg);
	Token GetCurrent() const;
	Token GetNext() const;
	Token GetPrevious() const;

private:
	const std::vector<Token>& m_Tokens;
	static const std::set<TokenType> s_BinaryTokenTypes;
	Context& m_Context;
	std::size_t m_Current {};
};

}
