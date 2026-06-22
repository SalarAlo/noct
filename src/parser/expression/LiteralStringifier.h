#pragma once

#include <format>
#include <string>
#include <unordered_set>
#include <variant>

#include "parser/expression/ClassInstance.h"
#include "parser/expression/ICallable.h"

#include "module/Module.h"

namespace Noct {

struct LiteralStringifier {
	int maxDepth = 3;

	mutable std::unordered_set<const void*> visited;

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
		return std::format("{}({} arguments)", f->Name(), f->Arity());
	}

	std::string operator()(const ModuleRef& m) const {
		return std::format("<module {}>", m->Name());
	}

	std::string operator()(const ClassInstanceRef& c) const {
		return stringifyInstance(c, 0);
	}

private:
	std::string stringifyValue(const NoctObject& v, int depth) const {
		if (depth >= maxDepth)
			return "...";
		return std::visit([&](const auto& x) -> std::string {
			using T = std::decay_t<decltype(x)>;
			if constexpr (std::is_same_v<T, ClassInstanceRef>) {
				return stringifyInstance(x, depth + 1);
			} else {
				return (*this)(x);
			}
		},
		    v);
	}

	std::string stringifyInstance(const ClassInstanceRef& c, int depth) const {
		const void* id = static_cast<const void*>(c.get());
		if (visited.contains(id)) {
			return std::format("<instance {} @{} (cycle)>",
			    c->ClassReference->Name,
			    std::format("{:p}", static_cast<const void*>(c.get())));
		}

		if (depth >= maxDepth) {
			return std::format("<instance {} @{}>",
			    c->ClassReference->Name,
			    std::format("{:p}", static_cast<const void*>(c.get())));
		}

		visited.insert(id);

		std::string out;
		out += std::format("instance {} @{}\n",
		    c->ClassReference->Name,
		    std::format("{:p}", static_cast<const void*>(c.get())));

		out += "fields: {\n";
		for (const auto& [name, val] : c->Fields) {
			out += "\t";
			out += name;
			out += ": ";
			out += stringifyValue(val, depth);
			out += "\n";
		}
		out += "}\n";

		out += "methods: {\n";
		for (const auto& [name, fn] : c->ClassReference->Methods) {
			out += "\t";
			out += name;
			out += "(";
			for (const auto& param : fn->ParameterNames) {
				out += param.Lexeme;
				out += ", ";
			}
			out += ");\n";
		}
		out += "}\n";

		visited.erase(id);
		return out;
	}
};

}
