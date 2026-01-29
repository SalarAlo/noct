#pragma once

#include <memory>

#include "noct/Context.h"
#include "noct/Environment.h"

#include "noct/lexer/NoctObject.h"
#include "noct/lexer/Token.h"

#include "noct/parser/expression/Expression.h"
#include "noct/parser/expression/ExpressionFwd.h"
#include "noct/parser/expression/FunctionValueFactory.h"
#include "noct/parser/expression/ICallable.h"

#include "noct/parser/expression/variants/Set.h"

#include "noct/parser/statement/ClassDecleration.h"
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
	void operator()(const This&);
	void operator()(const Ternary&);
	void operator()(const Grouping&);
	void operator()(const Variable&);
	void operator()(const Assign&);
	void operator()(const Logical&);
	void operator()(const Call&);
	void operator()(const Maybe&);
	void operator()(const Get&);
	void operator()(const Set&);
	void operator()(const Lambda&);

	void operator()(const ExpressionStatement&);
	void operator()(const PrintStatement&);
	void operator()(const VariableDecleration&);
	void operator()(const ClassDecleration&);
	void operator()(FunctionDecleration&);
	void operator()(const BlockStatement&);
	void operator()(const IfStatement&);
	void operator()(const WhileStatement&);
	void operator()(const ReturnStatement&);
	void operator()(const BreakStatement&);

	void SetGlobalEnvironment(const std::shared_ptr<Environment>& env);
	void Interpret(const StatementPtrVector& statements);

	NoctObject InvokeFunction(FunctionValue& fn, const std::vector<NoctObject>& args, const Token& callSite);

	bool IsEqual(const NoctObject& left, const NoctObject& right);

	void EnsureNumbers(const Token& op, double* operand);
	void EnsureNumbers(const Token& op, double* operand, double* operand2);

	NoctObject GetLiteral() const { return m_Value; }

	void Evaluate(Expression& exp);
	void Execute(Statement& exp);

private:
	struct EnvGuard {
		Interpreter& I;
		std::shared_ptr<Environment> saved;
		explicit EnvGuard(Interpreter& I, std::shared_ptr<Environment> next)
		    : I(I)
		    , saved(I.m_Env) { I.m_Env = std::move(next); }
		~EnvGuard() { I.m_Env = std::move(saved); }

		EnvGuard(const EnvGuard&) = delete;
		EnvGuard& operator=(const EnvGuard&) = delete;
	};

private:
	Context& m_Context;
	NoctObject m_Value {};
	std::shared_ptr<Environment> m_Env { nullptr };
};

}
