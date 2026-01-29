#pragma once

#include <cassert>

#include <optional>
#include <vector>

#include "noct/Environment.h"

#include "noct/lexer/Token.h"

#include "noct/parser/expression/ClassInstanceFwd.h"
#include "noct/parser/expression/FunctionValueFwd.h"

#include "noct/parser/statement/StatementFwd.h"

namespace Noct {

struct FunctionValue {
	const StatementPtrVector* Body = nullptr;
	std::vector<Token> ParameterNames {};
	std::optional<Token> Name {};
	size_t FrameSize = 0;
	size_t ClosureSize = 0;
	std::shared_ptr<Environment> Closure = nullptr;

	FunctionValue(const StatementPtrVector* body,
	    std::vector<Token> params,
	    std::optional<Token> name,
	    size_t frameSize,
	    size_t closureSize,
	    std::shared_ptr<Environment> closure)
	    : Body(body)
	    , ParameterNames(std::move(params))
	    , Name(std::move(name))
	    , FrameSize(frameSize)
	    , ClosureSize(closureSize)
	    , Closure(std::move(closure)) {
	}

	FunctionValueRef Bind(const ClassInstanceRef& ref);

	~FunctionValue();
};

using FunctionValueRef = std::shared_ptr<FunctionValue>;

inline bool operator==(const FunctionValue& a, const FunctionValue& b) {
	return a.Body == b.Body
	    && a.ParameterNames == b.ParameterNames
	    && a.Name == b.Name;
}

inline bool operator!=(const FunctionValue& a, const FunctionValue& b) {
	return !(a == b);
}

}
