#pragma once

#include <iostream>

#include "Visitor.h"
#include "noct/parser/Unary.h"
#include "noct/parser/Ternary.h"
#include "noct/parser/Binary.h"
#include "noct/parser/Literal.h"
#include "noct/parser/Grouping.h"

namespace Noct {

struct PrintVisitor final : Visitor {
	void Visit(const Unary& u) override {
		std::cout << "(" << u.Operator.Lexeme << " ";
		u.Right->Accept(*this);
		std::cout << ")";
	}

	void Visit(const Binary& b) override {
		std::cout << "(";
		b.Left->Accept(*this);
		std::cout << " " << b.Operator.Lexeme << " ";
		b.Right->Accept(*this);
		std::cout << ")";
	}

	void Visit(const Literal& l) override {
		std::visit([](auto& value) {
			using T = std::decay_t<decltype(value)>;
			if constexpr (std::is_same_v<T, std::monostate>) {
				std::cout << "nil";
			} else {
				std::cout << value;
			}
		},
		    l.Value);
	}

	void Visit(const Grouping& g) override {
		std::cout << "(";
		g.GroupExpr->Accept(*this);
		std::cout << ")";
	}

	void Visit(const Ternary& g) override {
		std::cout << "(";
		g.Condition->Accept(*this);
		std::cout << " ? ";
		g.Left->Accept(*this);
		std::cout << " : ";
		g.Right->Accept(*this);
		std::cout << ")";
	}
};

}
