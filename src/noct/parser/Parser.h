#pragma once

#include <memory>
#include <set>
#include <vector>
#include <initializer_list>

#include "noct/Context.h"
#include "noct/parser/Expression.h"
#include "noct/lexer/Token.h"
#include "noct/lexer/TokenType.h"

namespace Noct {

class Parser {
public:
	Parser(const std::vector<Token>& tokens, Context& ctx);
	std::unique_ptr<Expression> Parse();

private:
	std::unique_ptr<Expression> Expr();
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
