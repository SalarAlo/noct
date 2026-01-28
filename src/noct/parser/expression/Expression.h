#pragma once

#include <utility>
#include <variant>

#include "noct/parser/expression/expression_variants/Assign.h"
#include "noct/parser/expression/expression_variants/Binary.h"
#include "noct/parser/expression/expression_variants/Call.h"
#include "noct/parser/expression/expression_variants/Get.h"
#include "noct/parser/expression/expression_variants/Grouping.h"
#include "noct/parser/expression/expression_variants/Lambda.h"
#include "noct/parser/expression/expression_variants/Literal.h"
#include "noct/parser/expression/expression_variants/Logical.h"
#include "noct/parser/expression/expression_variants/Maybe.h"
#include "noct/parser/expression/expression_variants/Set.h"
#include "noct/parser/expression/expression_variants/Ternary.h"
#include "noct/parser/expression/expression_variants/Unary.h"
#include "noct/parser/expression/expression_variants/Variable.h"

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
	    Unary,
	    Variable>;

	Variant Value;

	template <class T>
	explicit Expression(T&& v)
	    : Value(std::forward<T>(v)) { }
};

}
