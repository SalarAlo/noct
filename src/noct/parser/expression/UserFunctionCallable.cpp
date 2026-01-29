#include "noct/parser/expression/UserFunctionCallable.h"

#include "noct/interpreter/Interpreter.h"

#include "noct/parser/expression/FunctionValue.h"

namespace Noct {

UserFunctionCallable::UserFunctionCallable(const FunctionValueRef& value)
    : m_FunctionValue(value) { }

size_t UserFunctionCallable::Arity() const { return m_FunctionValue->ParameterNames.size(); }

NoctObject UserFunctionCallable::Call(CallContext& ctx, const std::vector<NoctObject>& args) {
	return ctx.I.InvokeFunction(*m_FunctionValue, args, ctx.Paren);
}

}
