#pragma once

#include "noct/lexer/NoctObject.h"

#include "noct/parser/expression/ClassInstanceFwd.h"
#include "noct/parser/expression/FunctionValue.h"
#include "noct/parser/expression/ICallable.h"

namespace Noct {

struct BoundMethod : public ICallable {
public:
	BoundMethod(ClassInstanceRef receiver, FunctionValueRef fun);

	size_t Arity() const override;
	NoctObject Call(CallContext& ctx, const std::vector<NoctObject>& args) override;
	std::string_view Name() const override { return "<method>"; }

private:
	ClassInstanceRef m_Receiver;
	FunctionValueRef m_Function;
};

using BoundMethodRef = std::shared_ptr<BoundMethod>;

}
