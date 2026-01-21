#pragma once

#include <memory>
#include <vector>

#include "noct/Environment.h"
#include "noct/parser/statement/Statement.h"

#include "noct/parser/expression/Expression.h"

#include "noct/parser/Visitor.h"
#include "noct/lexer/Token.h"

namespace Noct {

class Interpreter final : public ExpressionVisitor, public StatementVisitor {
public:
	void Visit(const Unary&) override;
	void Visit(const Binary&) override;
	void Visit(const Literal&) override;
	void Visit(const Ternary&) override;
	void Visit(const Grouping&) override;
	void Visit(const Variable&) override;
	void Visit(const Assign&) override;

	void Visit(const ExpressionStatement&) override;
	void Visit(const PrintStatement&) override;
	void Visit(const VariableDecleration&) override;

	void Interpret(const std::vector<std::unique_ptr<Statement>>& statements);

	bool IsTruthy(const NoctLiteral& literal);
	bool IsEqual(const NoctLiteral& left, const NoctLiteral& right);

	void EnsureNumbers(const Token& op, double* operand);
	void EnsureNumbers(const Token& op, double* operand, double* operand2);

	NoctLiteral GetLiteral() const { return m_Value; }

	void Evaluate(const Expression& exp);
	void Execute(const Statement& exp);

private:
	NoctLiteral m_Value;
	Environment m_Env;
};

}
