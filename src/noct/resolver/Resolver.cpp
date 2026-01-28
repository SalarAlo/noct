#include "noct/resolver/Resolver.h"

#include "noct/Context.h"
#include "noct/Logger.h"

#include "noct/parser/expression/expression_variants/Get.h"

#include "noct/parser/statement/ClassDecleration.h"

namespace Noct {

Resolver::Resolver(Context& ctx)
    : m_Context(ctx) { }

void Resolver::Resolve(const StatementPtrVector& statements) {
	m_Scopes.clear();
	BeginScope();

	for (const auto& stmt : statements) {
		Resolve(*stmt);
	}

	m_GlobalLocalCount = GetCurrentScope().LocalCount;
	EndScope();
}

auto Resolver::GetCurrentScope() -> Scope& {
	return m_Scopes.back();
}

void Resolver::Resolve(Expression& expr) {
	std::visit(*this, expr.Value);
}

void Resolver::Resolve(Statement& stmt) {
	std::visit(*this, stmt.Instruction);
}

void Resolver::BeginScope() {
	m_Scopes.push_back({});
}

void Resolver::EndScope() {
	auto& scope { m_Scopes.back() };

	for (const auto& [name, md] : scope.Slots) {
		switch (md.Kind) {
		case SymbolKind::Parameter:
			[[fallthrough]];
		case SymbolKind::Variable:
			if (md.ReadCount == 0)
				Logger::Warn("Identifier {} is unused", name);
			break;

		case SymbolKind::Function:
			if (md.ReadCount == 0)
				Logger::Warn("Function {} is unused", name);
			break;

		case SymbolKind::Class:
			if (md.ReadCount == 0)
				Logger::Warn("Class {} is unused", name);
			break;
		}
	}

	m_Scopes.pop_back();
}

std::optional<size_t> Resolver::TryDeclareInCurrentScope(const Token& name, SymbolKind kind) {
	auto& scope = GetCurrentScope();

	if (scope.Slots.contains(name.Lexeme)) {
		m_Context.ReportResolveError(name, "Redeclared symbol in same scope");
		return {};
	}

	const size_t slot = scope.LocalCount++;
	scope.Slots[name.Lexeme].Slot = slot;
	scope.Slots[name.Lexeme].ReadCount = 0;
	scope.Slots[name.Lexeme].Defined = false;
	scope.Slots[name.Lexeme].Kind = kind;
	return slot;
}

bool Resolver::TryResolveVariableUse(Token& nameToken, size_t& outSlot, size_t& outDepth, bool isRead) {
	const size_t innermost = m_Scopes.size() - 1;

	for (size_t i = innermost;; --i) {
		auto& scope = m_Scopes[i];
		auto it = scope.Slots.find(nameToken.Lexeme);
		if (it != scope.Slots.end()) {
			if (!it->second.Defined && i == innermost) {
				m_Context.ReportResolveError(nameToken, "Variable used in its own initializer");
				outSlot = 0;
				outDepth = 0;
				return false;
			}

			outSlot = it->second.Slot;
			if (isRead)
				it->second.ReadCount++;
			outDepth = innermost - i;
			return true;
		}
		if (i == 0)
			break;
	}

	m_Context.ReportResolveError(nameToken, "Undefined variable access");

	outSlot = 0;
	outDepth = 0;

	return false;
}

void Resolver::operator()(Literal&) { }

void Resolver::operator()(Maybe&) { }

void Resolver::operator()(Grouping& g) {
	Resolve(*g.GroupExpr);
}

void Resolver::operator()(Unary& u) {
	Resolve(*u.Right);
}

void Resolver::operator()(Binary& b) {
	Resolve(*b.Left);
	Resolve(*b.Right);
}

void Resolver::operator()(Logical& l) {
	Resolve(*l.Left);
	Resolve(*l.Right);
}

void Resolver::operator()(Ternary& t) {
	Resolve(*t.Condition);
	Resolve(*t.Left);
	Resolve(*t.Right);
}

void Resolver::operator()(Call& c) {
	Resolve(*c.Callee);
	for (auto& arg : c.Arguments) {
		Resolve(*arg);
	}
}

void Resolver::operator()(Get& get) {
	Resolve(*get.Instance);
}

void Resolver::operator()(Set& get) {
	Resolve(*get.Value);
	Resolve(*get.Instance);
}

void Resolver::operator()(Variable& v) {
	TryResolveVariableUse(v.Name, v.Slot, v.Depth, true);
}

void Resolver::operator()(Assign& a) {
	TryResolveVariableUse(a.Name, a.Slot, a.Depth, false);
	Resolve(*a.Value);
}

void Resolver::operator()(Lambda& lambda) {
	const bool enclosingFunction = m_InFunction;
	m_InFunction = true;

	BeginScope();
	auto& lambdaScope = GetCurrentScope();

	for (const auto& param : lambda.Parameters) {
		if (lambdaScope.Slots.contains(param.Lexeme)) {
			m_Context.ReportResolveError(param, "Duplicate lambda parameter");
			continue;
		}

		const size_t paramSlot = lambdaScope.LocalCount++;

		auto& meta = lambdaScope.Slots[param.Lexeme];
		meta.Slot = paramSlot;
		meta.ReadCount = 0;
		meta.Defined = true;
		meta.Kind = SymbolKind::Parameter;
	}

	for (auto& stmt : lambda.Body) {
		Resolve(*stmt);
	}

	lambda.LocalCount = lambdaScope.LocalCount;
	EndScope();

	m_InFunction = enclosingFunction;
}

void Resolver::operator()(ExpressionStatement& s) {
	Resolve(*s.Expr);
}

void Resolver::operator()(PrintStatement& s) {
	Resolve(*s.PrintExpr);
}

void Resolver::operator()(VariableDecleration& d) {
	const auto slot = TryDeclareInCurrentScope(d.Name, SymbolKind::Variable);

	if (!slot) {
		return;
	}

	d.Slot = *slot;

	auto& meta = GetCurrentScope().Slots[d.Name.Lexeme];
	if (d.Initialiser) {
		Resolve(*d.Initialiser);
	}
	meta.Defined = true;
}

void Resolver::operator()(BlockStatement& b) {
	BeginScope();
	auto& blockScope = GetCurrentScope();

	for (auto& stmt : b.Statements) {
		Resolve(*stmt);
	}

	b.LocalCount = blockScope.LocalCount;
	EndScope();
}

void Resolver::operator()(IfStatement& i) {
	Resolve(*i.Condition);
	Resolve(*i.TrueStatement);
	if (i.FalseStatement) {
		Resolve(*i.FalseStatement);
	}
}

void Resolver::operator()(WhileStatement& w) {
	const bool enclosingLoop = m_InLoop;
	m_InLoop = true;

	Resolve(*w.Condition);
	Resolve(*w.LoopGuts);

	m_InLoop = enclosingLoop;
}

void Resolver::operator()(BreakStatement& b) {
	if (!m_InLoop) {
		m_Context.ReportResolveError(b.BreakToken, "calling break outside of loop");
	}
}

void Resolver::operator()(ReturnStatement& r) {
	if (!m_InFunction) {
		m_Context.ReportResolveError(r.ReturnToken, "Return used outside of Function");
	}

	if (r.ReturnExpr) {
		Resolve(*r.ReturnExpr);
	}
}

void Resolver::operator()(FunctionDecleration& fn) {
	const auto fnSlot = TryDeclareInCurrentScope(fn.Name, SymbolKind::Function);
	if (!fnSlot) {
		return;
	}
	fn.Slot = *fnSlot;

	auto& meta = GetCurrentScope().Slots[fn.Name.Lexeme];
	meta.Defined = true;

	const bool enclosingFunction = m_InFunction;
	m_InFunction = true;

	BeginScope();
	auto& fnScope = GetCurrentScope();

	for (const auto& param : fn.Parameters) {
		if (fnScope.Slots.contains(param.Lexeme)) {
			m_Context.ReportResolveError(param, "Duplicate function parameter");
		}
		const size_t paramSlot = fnScope.LocalCount++;
		auto& meta = fnScope.Slots[param.Lexeme];

		meta.Slot = paramSlot;
		meta.ReadCount = 0;
		meta.Kind = SymbolKind::Parameter;
		meta.Defined = true;
	}

	for (auto& stmt : fn.Body) {
		Resolve(*stmt);
	}

	fn.LocalCount = fnScope.LocalCount;
	EndScope();

	m_InFunction = enclosingFunction;
}

void Resolver::operator()(ClassDecleration& classDecl) {
	const auto classSlot = TryDeclareInCurrentScope(classDecl.Name, SymbolKind::Class);

	if (!classSlot) {
		return;
	}

	classDecl.Slot = *classSlot;
	auto& meta = GetCurrentScope().Slots[classDecl.Name.Lexeme];
	meta.Defined = true;

	for (auto& method : classDecl.Methods) {
		operator()(method);
	}
}

}
