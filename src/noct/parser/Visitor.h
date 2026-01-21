#pragma once

namespace Noct {
struct Unary;
struct Binary;
struct Literal;
struct Grouping;
struct Ternary;
struct Variable;
struct Assign;

struct ExpressionStatement;
struct PrintStatement;
struct VariableDecleration;

struct ExpressionVisitor {
	virtual ~ExpressionVisitor() = default;

	virtual void Visit(const Unary&) = 0;
	virtual void Visit(const Binary&) = 0;
	virtual void Visit(const Literal&) = 0;
	virtual void Visit(const Ternary&) = 0;
	virtual void Visit(const Grouping&) = 0;
	virtual void Visit(const Variable&) = 0;
	virtual void Visit(const Assign&) = 0;
};

struct StatementVisitor {
	virtual ~StatementVisitor() = default;

	virtual void Visit(const ExpressionStatement&) = 0;
	virtual void Visit(const PrintStatement&) = 0;
	virtual void Visit(const VariableDecleration&) = 0;
};

}
