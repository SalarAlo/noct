#include "noct/parser/expression/BoundMethod.h"

#include "noct/parser/expression/UserFunctionCallable.h"

namespace Noct {

BoundMethod::BoundMethod(ClassInstanceRef receiver, FunctionValueRef fun)
    : m_Function(std::move(fun))
    , m_Receiver(std::move(receiver)) {
}

size_t BoundMethod::Arity() const {
	return m_Function->ParameterNames.size();
}

NoctObject BoundMethod::Call(CallContext& ctx, const std::vector<NoctObject>& args) {
	auto bound = m_Function->Bind(m_Receiver);
	CallableRef callable = std::make_shared<UserFunctionCallable>(bound);
	return callable->Call(ctx, args);
}

}
