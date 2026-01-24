#pragma once

#include "noct/parser/expression/FunctionValue.h"
#include <format>
#include <string>
#include <variant>

namespace Noct {
struct LiteralStringifier {
	std::string operator()(const double& value) const {
		return std::format("{:g}", value);
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

	std::string operator()(const std::shared_ptr<FunctionValue>& f) const {
		std::string out {};
		out.append(f->Name.has_value() ? f->Name->Lexeme : "<ananoymous_function>");
		out.append("(");
		for (const auto& arg : f->ArgumentNames) {
			out.append(arg.Lexeme);
		}
		out.append(");");

		return out;
	}
};
}
