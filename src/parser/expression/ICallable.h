#pragma once

#include <vector>

#include "Embedding.h"
#include "lexer/NoctObject.h"
#include "lexer/Token.h"

namespace Noct {
class Interpreter;

struct CallContext {
	Interpreter& I;
	Token Paren;
	ScriptId CurrentScript {};
	std::filesystem::path CurrentFile {};
	std::vector<NoctDiagnostic>* Diagnostics {};
	std::vector<NoctComponentDeclaration>* Components {};
};

struct ICallable {
	virtual ~ICallable() = default;
	virtual size_t Arity() const = 0;
	virtual bool IsVariadic() const { return false; }
	virtual NoctObject Call(CallContext& ctx, const std::vector<NoctObject>& args) = 0;
	virtual std::string_view Name() const = 0;
};

}
