#pragma once

#include <fmt/format.h>

#include <unordered_map>

#include "noct/exceptions/RuntimeException.h"

#include "noct/lexer/NoctObject.h"
#include "noct/lexer/Token.h"

#include "noct/parser/expression/ClassValue.h"

namespace Noct {

struct ClassInstance {
	ClassValueRef ClassReference;
	std::unordered_map<std::string, NoctObject> Fields {};

	explicit ClassInstance(ClassValueRef cls)
	    : ClassReference(std::move(cls)) {
		if (!ClassReference) {
			throw std::runtime_error("ClassInstance created with null ClassReference");
		}
	}

	NoctObject Get(const Token& name) {

		if (!ClassReference) {
			throw RuntimeError(name, "Internal error: ClassInstance has no ClassReference");
		}

		if (auto iter { Fields.find(name.Lexeme) }; iter != Fields.end()) {
			return iter->second;
		}

		if (auto iter = ClassReference->Methods.find(name.Lexeme); iter != ClassReference->Methods.end()) {
			return iter->second;
		}

		throw RuntimeError(name, fmt::format("Undefined property {}", name.Lexeme));
	}

	void Set(const Token& name, const NoctObject& obj) {
		if (!ClassReference) {
			throw RuntimeError(name, "Internal error: ClassInstance has no ClassReference");
		}

		if (auto iter = ClassReference->Methods.find(name.Lexeme); iter != ClassReference->Methods.end()) {
			throw RuntimeError(name, fmt::format("Trying set change instance methods '{}'", name.Lexeme));
		}

		Fields[name.Lexeme] = obj;
	}

	bool operator==(const ClassInstance& b) {
		return false;
	}

	bool operator!=(const ClassInstance& b) {
		return true;
	}
};
}
