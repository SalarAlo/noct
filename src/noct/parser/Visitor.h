#pragma once

namespace Noct {
struct Unary;
struct Binary;
struct Literal;
struct Grouping;

struct Visitor {
	virtual ~Visitor() = default;

	virtual void Visit(const Unary&) = 0;
	virtual void Visit(const Binary&) = 0;
	virtual void Visit(const Literal&) = 0;
	virtual void Visit(const Grouping&) = 0;
};

}
