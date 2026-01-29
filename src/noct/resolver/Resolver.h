#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "noct/Context.h"

#include "noct/lexer/Token.h"

#include "noct/parser/expression/Expression.h"
#include "noct/parser/expression/ExpressionFwd.h"

#include "noct/parser/expression/variants/Assign.h"
#include "noct/parser/expression/variants/Binary.h"
#include "noct/parser/expression/variants/Call.h"
#include "noct/parser/expression/variants/Grouping.h"
#include "noct/parser/expression/variants/Lambda.h"
#include "noct/parser/expression/variants/Literal.h"
#include "noct/parser/expression/variants/Logical.h"
#include "noct/parser/expression/variants/Maybe.h"
#include "noct/parser/expression/variants/Ternary.h"
#include "noct/parser/expression/variants/Unary.h"
#include "noct/parser/expression/variants/Variable.h"

#include "noct/parser/statement/BlockStatement.h"
#include "noct/parser/statement/BreakStatement.h"
#include "noct/parser/statement/ClassDecleration.h"
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
	Resolver(Context& ctx);
	void Resolve(const StatementPtrVector& statements);

	size_t GetGlobalFrameSize() const { return m_GlobalFrameSize; }

	void operator()(Assign&);
	void operator()(Binary&);
	void operator()(Call&);
	void operator()(Get&);
	void operator()(Grouping&);
	void operator()(Lambda&);
	void operator()(Literal&);
	void operator()(Logical&);
	void operator()(Maybe&);
	void operator()(Set&);
	void operator()(Ternary&);
	void operator()(This&);
	void operator()(Unary&);
	void operator()(Variable&);

	void operator()(ExpressionStatement&);
	void operator()(PrintStatement&);
	void operator()(VariableDecleration&);
	void operator()(FunctionDecleration&);
	void operator()(ClassDecleration&);
	void operator()(BlockStatement&);
	void operator()(IfStatement&);
	void operator()(WhileStatement&);
	void operator()(ReturnStatement&);
	void operator()(BreakStatement&);

private:
	enum class SymbolKind {
		Variable,
		Function,
		Class,
		Parameter
	};

	struct ScopeGuard {
		Resolver& R;
		explicit ScopeGuard(Resolver& r)
		    : R(r) { R.BeginScope(); }
		~ScopeGuard() { R.EndScope(); }
	};

	struct SlotMeta {
		size_t ReadCount {};
		size_t Slot {};
		bool Defined {};
		SymbolKind Kind {};
	};

	struct Scope {
		std::unordered_map<std::string, SlotMeta> Slots;
		size_t NextSlot {};
	};

private:
	Scope& GetCurrentScope();

	void Resolve(Expression& expr);
	void Resolve(Statement& stmt);

	void BeginScope();
	void EndScope();

	void ResolveMethod(FunctionDecleration& fn);

	std::optional<size_t> TryDeclareInCurrentScope(const Token& name, SymbolKind kind);

	bool TryResolveVariableUse(Token& nameToken, size_t& outSlot, size_t& outDepth, bool isRead);
	void ResolveCallableBody(std::vector<Token>& params, StatementPtrVector& body, size_t& outFrameSize, bool injectThis, size_t& outClosureSize);

private:
	Context& m_Context;
	std::vector<Scope> m_Scopes;

	bool m_InFunction {};
	bool m_InClass {};
	bool m_InLoop {};

	size_t m_GlobalFrameSize {};
};
}
