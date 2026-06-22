#pragma once

#include "lexer/NoctObject.h"

#include "parser/expression/ClassInstanceFwd.h"
#include "parser/expression/FunctionValue.h"
#include "parser/expression/ICallable.h"

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
