
#pragma once

#include <string>
#include <variant>

namespace Noct {
struct LiteralTypeStringifier {
	std::string operator()(const double& _) const {
		return "double";
	}

	std::string operator()(const std::string& _) const {
		return "string";
	}
	std::string operator()(const bool& _) const {
		return "bool";
	}

	std::string operator()(const std::monostate& _) const {
		return "nil";
	}
};
}
