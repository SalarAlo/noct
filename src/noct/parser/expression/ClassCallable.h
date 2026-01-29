#pragma once

#include "noct/parser/expression/ClassValue.h"
#include "noct/parser/expression/FunctionValueFwd.h"
#include "noct/parser/expression/ICallable.h"

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
