#pragma once

#include "parser/expression/ClassValue.h"
#include "parser/expression/FunctionValueFwd.h"
#include "parser/expression/ICallable.h"

namespace Noct {
class ClassCallable : public ICallable {
public:
	ClassCallable(const ClassValueRef& ref);

	size_t Arity() const override;
	NoctObject Call(CallContext& ctx, const std::vector<NoctObject>& args) override;
	std::string_view Name() const override { return "<constructor>"; }

private:
	ClassValueRef m_ClassReference;
	FunctionValueRef m_InitMethod;
};

}
