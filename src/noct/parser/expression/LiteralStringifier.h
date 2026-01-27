#pragma once

#include <format>
#include <string>
#include <variant>

#include "noct/parser/expression/ClassInstance.h"
#include "noct/parser/expression/ClassValue.h"
#include "noct/parser/expression/FunctionValue.h"

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

	std::string operator()(const FunctionValueRef& f) const {
		std::string out {};
		out.append(f->Name.has_value() ? f->Name->Lexeme : "<ananoymous_function>");
		out.append("(");
		for (const auto& arg : f->ParameterNames) {
			out.append(arg.Lexeme);
			out.append(", ");
		}
		out.append(");");

		return out;
	}

	std::string operator()(const ClassValueRef& c) const {
		std::string out {};
		out.append("class ").append(c->Name);
		return out;
	}

	std::string operator()(const ClassInstance& c) const {
		std::string out {};
		out.append("instance of class ").append(c.ClassReference->Name);
		return out;
	}
};

}
