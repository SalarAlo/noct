#pragma once

#include <charconv>
#include <string>
#include <variant>

namespace Noct {
struct LiteralStringifier {
	std::string operator()(const double& value) const {
		char buffer[64];
		auto [ptr, ec] = std::to_chars(
		    buffer,
		    buffer + sizeof(buffer),
		    value,
		    std::chars_format::general);

		return std::string(buffer, ptr);
	}

	std::string operator()(const std::string& value) const {
		return "\"" + value + "\"";
	}
	std::string operator()(const bool& value) const {
		return value ? "true" : "false";
	}

	std::string operator()(const std::monostate&) const {
		return "nil";
	}
};
}
