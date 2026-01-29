#pragma once

#include "noct/parser/expression/FunctionValueFwd.h"
#include "noct/parser/expression/ICallable.h"

namespace Noct {
class UserFunctionCallable : public ICallable {
public:
	UserFunctionCallable(const FunctionValueRef& value);
	size_t Arity() const override;
	NoctObject Call(CallContext& ctx, const std::vector<NoctObject>& args) override;
	std::string_view Name() const override { return "<fn>"; }

private:
	FunctionValueRef m_FunctionValue;
};

}
