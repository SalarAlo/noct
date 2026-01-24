#pragma once

#include "noct/lexer/Token.h"
#include "noct/parser/statement/StatementFwd.h"

#include <optional>
#include <vector>

namespace Noct {

struct FunctionValue {
	std::vector<StatementPtr> Body;
	std::vector<Token> ArgumentNames;
	std::optional<Token> Name;

	~FunctionValue();
};

inline bool operator==(const FunctionValue& a, const FunctionValue& b) {
	return a.Body == b.Body
	    && a.ArgumentNames == b.ArgumentNames
	    && a.Name == b.Name;
}

inline bool operator!=(const FunctionValue& a, const FunctionValue& b) {
	return !(a == b);
}

}
