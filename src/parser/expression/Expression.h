#pragma once

#include <utility>
#include <variant>

#include "parser/expression/variants/Assign.h"
#include "parser/expression/variants/Binary.h"
#include "parser/expression/variants/Call.h"
#include "parser/expression/variants/Get.h"
#include "parser/expression/variants/Grouping.h"
#include "parser/expression/variants/Lambda.h"
#include "parser/expression/variants/Literal.h"
#include "parser/expression/variants/Logical.h"
#include "parser/expression/variants/Maybe.h"
#include "parser/expression/variants/Set.h"
#include "parser/expression/variants/Ternary.h"
#include "parser/expression/variants/This.h"
#include "parser/expression/variants/Unary.h"
#include "parser/expression/variants/Variable.h"

namespace Noct {

struct Expression {
	using Variant = std::variant<
	    Assign,
	    Binary,
	    Call,
	    Get,
	    Grouping,
	    Lambda,
	    Literal,
	    Logical,
	    Maybe,
	    Set,
	    Ternary,
	    This,
	    Unary,
	    Variable>;

	Variant Value;

	template <class T>
	explicit Expression(T&& v)
	    : Value(std::forward<T>(v)) { }

	Expression(const Expression&) = delete;
	Expression& operator=(const Expression&) = delete;

	Expression(Expression&&) noexcept = default;
	Expression& operator=(Expression&&) noexcept = default;
};

}
