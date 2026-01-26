#pragma once

#include <cstddef>
#include <string>
#include <unordered_map>
#include <vector>

#include "noct/lexer/Token.h"

#include "noct/parser/expression/Expression.h"
#include "noct/parser/expression/ExpressionFwd.h"

#include "noct/parser/expression/expression_variants/Assign.h"
#include "noct/parser/expression/expression_variants/Binary.h"
#include "noct/parser/expression/expression_variants/Call.h"
#include "noct/parser/expression/expression_variants/Grouping.h"
#include "noct/parser/expression/expression_variants/Lambda.h"
#include "noct/parser/expression/expression_variants/Literal.h"
#include "noct/parser/expression/expression_variants/Logical.h"
#include "noct/parser/expression/expression_variants/Maybe.h"
#include "noct/parser/expression/expression_variants/Ternary.h"
#include "noct/parser/expression/expression_variants/Unary.h"
#include "noct/parser/expression/expression_variants/Variable.h"

#include "noct/parser/statement/BlockStatement.h"
#include "noct/parser/statement/BreakStatement.h"
#include "noct/parser/statement/ExpressionStatement.h"
#include "noct/parser/statement/FunctionDecleration.h"
#include "noct/parser/statement/IfStatement.h"
#include "noct/parser/statement/PrintStatement.h"
#include "noct/parser/statement/ReturnStatement.h"
#include "noct/parser/statement/Statement.h"
#include "noct/parser/statement/StatementFwd.h"
#include "noct/parser/statement/VariableDecleration.h"
#include "noct/parser/statement/WhileStatement.h"

namespace Noct {

class Resolver final {
public:
	void Resolve(const StatementPtrVector& statements);

	size_t GetGlobalLocalCount() const { return m_GlobalLocalCount; }

	void operator()(Literal&);
	void operator()(Variable&);
	void operator()(Assign&);
	void operator()(Binary&);
	void operator()(Unary&);
	void operator()(Logical&);
	void operator()(Call&);
	void operator()(Lambda&);
	void operator()(Grouping&);
	void operator()(Ternary&);
	void operator()(Maybe&);

	void operator()(ExpressionStatement&);
	void operator()(PrintStatement&);
	void operator()(VariableDecleration&);
	void operator()(FunctionDecleration&);
	void operator()(BlockStatement&);
	void operator()(IfStatement&);
	void operator()(WhileStatement&);
	void operator()(ReturnStatement&);
	void operator()(BreakStatement&);

private:
	struct VariableData {
		size_t ReadCount {};
		size_t Slot {};
	};

	struct Scope {
		std::unordered_map<std::string, VariableData> Slots;
		size_t LocalCount = 0;
	};

private:
	Scope& GetCurrentScope();

	void Resolve(Expression& expr);
	void Resolve(Statement& stmt);

	void BeginScope();
	void EndScope();

	size_t DeclareInCurrentScope(const Token& name);

	void ResolveVariableUse(Token& nameToken, size_t& outSlot, size_t& outDepth, bool isRead);

private:
	std::vector<Scope> m_Scopes;

	bool m_InFunction = false;
	bool m_InLoop = false;

	size_t m_GlobalLocalCount = 0;
};

}
