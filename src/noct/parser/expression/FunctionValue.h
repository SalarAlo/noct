#pragma once

#include <optional>
#include <vector>

#include "noct/lexer/Token.h"

#include "noct/parser/statement/StatementFwd.h"

namespace Noct {
struct Environment;

struct FunctionValue {
	const StatementPtrVector* Body;
	std::vector<Token> ParameterNames;
	std::optional<Token> Name;

	Environment* Closure;
	~FunctionValue();
};

using FunctionRef = std::shared_ptr<FunctionValue>;

inline bool operator==(const FunctionValue& a, const FunctionValue& b) {
	return a.Body == b.Body
	    && a.ParameterNames == b.ParameterNames
	    && a.Name == b.Name;
}

inline bool operator!=(const FunctionValue& a, const FunctionValue& b) {
	return !(a == b);
}

}
