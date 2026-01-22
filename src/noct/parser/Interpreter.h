#pragma once

#include <memory>
#include <vector>

#include "noct/Context.h"
#include "noct/Environment.h"
#include "noct/parser/statement/Statement.h"

#include "noct/parser/expression/Expression.h"

#include "noct/parser/Visitor.h"
#include "noct/lexer/Token.h"

namespace Noct {

class Interpreter final : public ExpressionVisitor, public StatementVisitor {
public:
	Interpreter(Context& context)
	    : m_Context(context) { };

	void Visit(const Unary&) override;
	void Visit(const Binary&) override;
	void Visit(const Literal&) override;
	void Visit(const Ternary&) override;
	void Visit(const Grouping&) override;
	void Visit(const Variable&) override;
	void Visit(const Assign&) override;
	void Visit(const Logical&) override;

	void Visit(const ExpressionStatement&) override;
	void Visit(const PrintStatement&) override;
	void Visit(const VariableDecleration&) override;
	void Visit(const BlockStatement&) override;
	void Visit(const IfStatement&) override;
	void Visit(const WhileStatement&) override;

	void Interpret(const std::vector<std::unique_ptr<Statement>>& statements);

	bool IsEqual(const NoctLiteral& left, const NoctLiteral& right);

	void EnsureNumbers(const Token& op, double* operand);
	void EnsureNumbers(const Token& op, double* operand, double* operand2);

	NoctLiteral GetLiteral() const { return m_Value; }

	void Evaluate(const Expression& exp);
	void Execute(const Statement& exp);

private:
	Context& m_Context;
	NoctLiteral m_Value {};
	std::unique_ptr<Environment> m_Env { std::make_unique<Environment>() };
};

}
