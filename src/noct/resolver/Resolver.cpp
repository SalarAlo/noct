#include "noct/resolver/Resolver.h"

namespace Noct {

void Resolver::Resolve(const StatementPtrVector& statements) {
	m_Scopes.clear();
	m_Scopes.push_back({});

	for (const auto& stmt : statements) {
		Resolve(*stmt);
	}

	m_GlobalLocalCount = m_Scopes.front().LocalCount;
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
	if (m_Scopes.size() <= 1) {
		throw std::runtime_error("Resolver: attempted to pop global scope");
	}
	m_Scopes.pop_back();
}

size_t Resolver::DeclareInCurrentScope(const Token& name) {
	auto& scope = GetCurrentScope();
	if (scope.Slots.contains(name.Lexeme)) {
		throw std::runtime_error("Redeclared symbol in same scope: " + name.Lexeme);
	}
	const size_t slot = scope.LocalCount++;
	scope.Slots[name.Lexeme] = slot;
	return slot;
}

void Resolver::ResolveVariableUse(Token& nameToken, size_t& outSlot, size_t& outDepth) {
	const size_t innermost = m_Scopes.size() - 1;

	for (size_t i = innermost;; --i) {
		auto& scope = m_Scopes[i];
		auto it = scope.Slots.find(nameToken.Lexeme);
		if (it != scope.Slots.end()) {
			outSlot = it->second;
			outDepth = innermost - i; // hops from current
			return;
		}
		if (i == 0)
			break;
	}

	throw std::runtime_error("Undefined variable access: " + nameToken.Lexeme);
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

void Resolver::operator()(Variable& v) {
	ResolveVariableUse(v.Name, v.Slot, v.Depth);
}

void Resolver::operator()(Assign& a) {
	ResolveVariableUse(a.Name, a.Slot, a.Depth);
	Resolve(*a.Value);
}

void Resolver::operator()(Lambda& lambda) {
	const bool enclosingFunction = m_InFunction;
	m_InFunction = true;

	BeginScope();
	auto& lambdaScope = GetCurrentScope();

	for (const auto& param : lambda.Parameters) {
		if (lambdaScope.Slots.contains(param.Lexeme)) {
			throw std::runtime_error("Duplicate lambda parameter: " + param.Lexeme);
		}
		const size_t paramSlot = lambdaScope.LocalCount++;
		lambdaScope.Slots[param.Lexeme] = paramSlot;
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
	const size_t slot = DeclareInCurrentScope(d.Name);
	d.Slot = slot;

	if (d.Initialiser) {
		Resolve(*d.Initialiser);
	}
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

void Resolver::operator()(BreakStatement&) {
	if (!m_InLoop) {
		throw std::runtime_error("break used outside of loop");
	}
}

void Resolver::operator()(ReturnStatement& r) {
	if (!m_InFunction) {
		throw std::runtime_error("return used outside of function");
	}
	if (r.ReturnExpr) {
		Resolve(*r.ReturnExpr);
	}
}

void Resolver::operator()(FunctionDecleration& fn) {
	const size_t fnSlot = DeclareInCurrentScope(fn.Name);
	fn.Slot = fnSlot;

	const bool enclosingFunction = m_InFunction;
	m_InFunction = true;

	BeginScope();
	auto& fnScope = GetCurrentScope();

	for (const auto& param : fn.Parameters) {
		if (fnScope.Slots.contains(param.Lexeme)) {
			throw std::runtime_error("Duplicate function parameter: " + param.Lexeme);
		}
		const size_t paramSlot = fnScope.LocalCount++;
		fnScope.Slots[param.Lexeme] = paramSlot;
	}

	for (auto& stmt : fn.Body) {
		Resolve(*stmt);
	}

	fn.LocalCount = fnScope.LocalCount;
	EndScope();

	m_InFunction = enclosingFunction;
}

}
