#pragma once

#include <string>
#include <variant>

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
};
}
