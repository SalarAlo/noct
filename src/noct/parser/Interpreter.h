#pragma once

#include <memory>
#include <vector>

#include "noct/Context.h"
#include "noct/Environment.h"

#include "noct/parser/expression/Expression.h"
#include "noct/parser/expression/ExpressionDef.h"

#include "noct/lexer/Token.h"
#include "noct/parser/statement/Statement.h"

namespace Noct {

class Interpreter final {
public:
	Interpreter(Context& context)
	    : m_Context(context) { };

	void operator()(const Unary&);
	void operator()(const Binary&);
	void operator()(const Literal&);
	void operator()(const Ternary&);
	void operator()(const Grouping&);
	void operator()(const Variable&);
	void operator()(const Assign&);
	void operator()(const Logical&);

	void operator()(const ExpressionStatement&);
	void operator()(const PrintStatement&);
	void operator()(const VariableDecleration&);
	void operator()(const BlockStatement&);
	void operator()(const IfStatement&);
	void operator()(const WhileStatement&);
	void operator()(const BreakStatement&);

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
