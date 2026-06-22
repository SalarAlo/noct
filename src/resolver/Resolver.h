#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "Context.h"

#include "lexer/Token.h"

#include "parser/expression/Expression.h"
#include "parser/expression/ExpressionFwd.h"

#include "parser/expression/variants/Assign.h"
#include "parser/expression/variants/Binary.h"
#include "parser/expression/variants/Call.h"
#include "parser/expression/variants/Grouping.h"
#include "parser/expression/variants/Lambda.h"
#include "parser/expression/variants/Literal.h"
#include "parser/expression/variants/Logical.h"
#include "parser/expression/variants/Maybe.h"
#include "parser/expression/variants/Ternary.h"
#include "parser/expression/variants/Unary.h"
#include "parser/expression/variants/Variable.h"

#include "parser/statement/BlockStatement.h"
#include "parser/statement/BreakStatement.h"
#include "parser/statement/ClassDecleration.h"
#include "parser/statement/ExpressionStatement.h"
#include "parser/statement/FunctionDecleration.h"
#include "parser/statement/IfStatement.h"
#include "parser/statement/ImportStatement.h"
#include "parser/statement/PrintStatement.h"
#include "parser/statement/ReturnStatement.h"
#include "parser/statement/Statement.h"
#include "parser/statement/StatementFwd.h"
#include "parser/statement/VariableDecleration.h"
#include "parser/statement/WhileStatement.h"

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
	void operator()(ImportStatement&);
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
		Module,
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
