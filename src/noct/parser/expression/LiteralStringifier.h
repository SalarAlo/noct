#pragma once

#include <format>
#include <string>
#include <variant>

#include "noct/parser/expression/ClassInstance.h"
#include "noct/parser/expression/ICallable.h"

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

	std::string operator()(const CallableRef& f) const {
		std::string out {};
		out.append(f->Name());
		out.append("(").append(std::to_string(f->Arity())).append(" arguments)");
		return out;
	}

	std::string operator()(const ClassInstanceRef& c) const {
		std::string out {};
		out.append("instance ").append(c->ClassReference->Name).append("\n");
		out.append("fields: {\n");
		for (const auto& [name, val] : c->Fields) {
			out.append("\t").append(name).append(": ").append(std::visit(*this, val)).append("\n");
		}
		out.append("}\n");

		out.append("methods: {\n");
		for (const auto& [name, val] : c->ClassReference->Methods) {
			out.append("\t").append(name).append("(");

			for (const auto& param : val->ParameterNames)
				out.append(param.Lexeme).append(", ");

			out.append(");\n");
		}
		out.append("}");
		return out;
	}
};

}
