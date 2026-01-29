#pragma once

#include <utility>
#include <variant>

#include "noct/parser/expression/variants/Assign.h"
#include "noct/parser/expression/variants/Binary.h"
#include "noct/parser/expression/variants/Call.h"
#include "noct/parser/expression/variants/Get.h"
#include "noct/parser/expression/variants/Grouping.h"
#include "noct/parser/expression/variants/Lambda.h"
#include "noct/parser/expression/variants/Literal.h"
#include "noct/parser/expression/variants/Logical.h"
#include "noct/parser/expression/variants/Maybe.h"
#include "noct/parser/expression/variants/Set.h"
#include "noct/parser/expression/variants/Ternary.h"
#include "noct/parser/expression/variants/This.h"
#include "noct/parser/expression/variants/Unary.h"
#include "noct/parser/expression/variants/Variable.h"

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
};

}
