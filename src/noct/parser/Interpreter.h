#pragma once

#include "noct/parser/Expression.h"
#include "noct/parser/Visitor.h"
#include "noct/lexer/Token.h"

namespace Noct {

class Interpreter : public Visitor {
public:
	void Visit(const Unary&) override;
	void Visit(const Binary&) override;
	void Visit(const Literal&) override;
	void Visit(const Ternary&) override;
	void Visit(const Grouping&) override;

	void Interpret(const Expression& exp);

	void Evaluate(const Expression& exp);
	bool IsTruthy(const NoctLiteral& literal);
	bool IsEqual(const NoctLiteral& left, const NoctLiteral& right);

	void EnsureNumbers(const Token& op, double* operand);
	void EnsureNumbers(const Token& op, double* operand, double* operand2);

	NoctLiteral GetLiteral() const { return m_Value; }

	std::string Stringify(const NoctLiteral& literal) const;

private:
	NoctLiteral m_Value;
};

}
