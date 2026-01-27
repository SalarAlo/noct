#pragma once

#include <string>
#include <variant>

#include "noct/parser/expression/ClassInstance.h"
#include "noct/parser/expression/ClassValue.h"
#include "noct/parser/expression/FunctionValue.h"

namespace Noct {
struct LiteralBoolifier {
	bool operator()(const double& value) const {
		return value != 0;
	}

	bool operator()(const std::string& value) const {
		return !value.empty();
	}

	bool operator()(const bool& value) const {
		return value;
	}

	bool operator()(const std::monostate&) const {
		return false;
	}

	bool operator()(const FunctionValueRef&) const {
		return true;
	}

	bool operator()(const ClassValueRef&) const {
		return true;
	}

	bool operator()(const ClassInstance&) const {
		return true;
	}
};
}
