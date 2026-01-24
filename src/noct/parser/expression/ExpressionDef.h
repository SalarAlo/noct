#pragma once

#include <variant>
#include <utility>

#include "noct/parser/expression/Unary.h"
#include "noct/parser/expression/Binary.h"
#include "noct/parser/expression/Literal.h"
#include "noct/parser/expression/Variable.h"
#include "noct/parser/expression/Assign.h"
#include "noct/parser/expression/Logical.h"
#include "noct/parser/expression/Grouping.h"
#include "noct/parser/expression/Ternary.h"
#include "noct/parser/expression/Call.h"

namespace Noct {

struct Expression {
	using Variant = std::variant<
	    Unary,
	    Binary,
	    Literal,
	    Variable,
	    Assign,
	    Logical,
	    Grouping,
	    Ternary,
	    Call>;

	Variant Value;

	template <class T>
	explicit Expression(T&& v)
	    : Value(std::forward<T>(v)) { }
};

}
