#pragma once

#include <vector>

#include "noct/lexer/NoctObject.h"
#include "noct/lexer/Token.h"

namespace Noct {
class Interpreter;

struct CallContext {
	Interpreter& I;
	Token Paren;
};

struct ICallable {
	virtual ~ICallable() = default;
	virtual size_t Arity() const = 0;
	virtual NoctObject Call(CallContext& ctx, const std::vector<NoctObject>& args) = 0;
	virtual std::string_view Name() const = 0;
};

}
