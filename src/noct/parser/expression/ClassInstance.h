#pragma once

#include <fmt/format.h>

#include <unordered_map>

#include "noct/exceptions/RuntimeException.h"

#include "noct/lexer/NoctObject.h"
#include "noct/lexer/Token.h"

#include "noct/parser/expression/BoundMethod.h"
#include "noct/parser/expression/ClassInstanceFwd.h"
#include "noct/parser/expression/ClassValue.h"

namespace Noct {

struct ClassInstance : std::enable_shared_from_this<ClassInstance> {
	ClassValueRef ClassReference;
	std::unordered_map<std::string, NoctObject> Fields {};

	explicit ClassInstance(ClassValueRef cls)
	    : ClassReference(std::move(cls)) {
		if (!ClassReference) {
			throw std::runtime_error("ClassInstance created with null ClassReference");
		}
	}

	NoctObject Get(const Token& name) {
		if (auto field = Fields.find(name.Lexeme); field != Fields.end())
			return field->second;

		if (auto method = ClassReference->Methods.find(name.Lexeme);
		    method != ClassReference->Methods.end()) {

			CallableRef bound = std::static_pointer_cast<ICallable>(std::make_shared<BoundMethod>(shared_from_this(), method->second));
			return NoctObject { bound };
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
