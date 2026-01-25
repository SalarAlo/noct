#pragma once

#include <memory>

#include "noct/Context.h"
#include "noct/Environment.h"

#include "noct/lexer/Token.h"

#include "noct/parser/expression/Expression.h"
#include "noct/parser/expression/ExpressionFwd.h"
#include "noct/parser/expression/Lambda.h"

#include "noct/parser/statement/FunctionDecleration.h"
#include "noct/parser/statement/ReturnStatement.h"
#include "noct/parser/statement/Statement.h"
#include "noct/parser/statement/StatementFwd.h"

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
	void operator()(const Call&);
	void operator()(const Maybe&);
	void operator()(Lambda&);

	void operator()(const ExpressionStatement&);
	void operator()(const PrintStatement&);
	void operator()(const VariableDecleration&);
	void operator()(FunctionDecleration&);
	void operator()(const BlockStatement&);
	void operator()(const IfStatement&);
	void operator()(const WhileStatement&);
	void operator()(const ReturnStatement&);
	void operator()(const BreakStatement&);

	void Interpret(const StatementPtrVector& statements);
	void HoistFunctions(const StatementPtrVector& statements);

	bool IsEqual(const NoctObject& left, const NoctObject& right);

	void EnsureNumbers(const Token& op, double* operand);
	void EnsureNumbers(const Token& op, double* operand, double* operand2);

	NoctObject GetLiteral() const { return m_Value; }

	void Evaluate(Expression& exp);
	void Execute(Statement& exp);

private:
	Context& m_Context;
	NoctObject m_Value {};
	std::unique_ptr<Environment> m_Env { std::make_unique<Environment>() };
};

}
