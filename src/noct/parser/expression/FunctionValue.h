#pragma once

#include <optional>
#include <vector>

#include "noct/lexer/Token.h"

#include "noct/parser/statement/StatementFwd.h"

namespace Noct {

struct FunctionValue {
	StatementPtrVector Body;
	std::vector<Token> ArgumentNames;
	std::optional<Token> Name;

	~FunctionValue();
};

using FunctionRef = std::shared_ptr<FunctionValue>;

inline bool operator==(const FunctionValue& a, const FunctionValue& b) {
	return a.Body == b.Body
	    && a.ArgumentNames == b.ArgumentNames
	    && a.Name == b.Name;
}

inline bool operator!=(const FunctionValue& a, const FunctionValue& b) {
	return !(a == b);
}

}
