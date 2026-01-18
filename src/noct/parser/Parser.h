#pragma once

#include <memory>
#include <vector>
#include <initializer_list>

#include "noct/Context.h"
#include "noct/parser/Expression.h"
#include "noct/scanner/Token.h"
#include "noct/scanner/TokenType.h"

namespace Noct {

class Parser {
public:
	Parser(const std::vector<Token>& tokens, Context& ctx);

	std::unique_ptr<Expression> Parse();

private:
	std::unique_ptr<Expression> Expr();
	std::unique_ptr<Expression> Equality();
	std::unique_ptr<Expression> Comparison();
	std::unique_ptr<Expression> Term();
	std::unique_ptr<Expression> Factor();
	std::unique_ptr<Expression> Unary();
	std::unique_ptr<Expression> Primary();

	bool MatchAny(std::initializer_list<TokenType> types);
	bool MatchCurrent(TokenType type) const;
	bool IsAtEnd() const;

	Token Advance();
	Token Consume(TokenType type, std::string_view msg);
	Token GetCurrent() const;
	Token GetPrevious() const;

private:
	const std::vector<Token>& m_Tokens;
	Context& m_Context;
	std::size_t m_Current {};
};

}
