#include "noct/parser/expression/ClassCallable.h"

#include "noct/parser/expression/BoundMethod.h"
#include "noct/parser/expression/ClassInstance.h"
#include "noct/parser/expression/ClassInstanceFwd.h"
#include "noct/parser/expression/FunctionValue.h"

namespace Noct {

ClassCallable::ClassCallable(const ClassValueRef& ref)
    : m_ClassReference(ref) {
	if (auto initIter { m_ClassReference->Methods.find("init") }; initIter != m_ClassReference->Methods.end()) {
		m_InitMethod = initIter->second;
	}
}

size_t ClassCallable::Arity() const {
	if (m_InitMethod)
		return m_InitMethod->ParameterNames.size();
	return 0;
}

NoctObject ClassCallable::Call(CallContext& ctx, const std::vector<NoctObject>& args) {
	auto instance { std::make_shared<ClassInstance>(m_ClassReference) };
	if (m_InitMethod) {
		auto bound = std::make_shared<BoundMethod>(instance, m_InitMethod);
		CallContext initCtx { ctx.I, ctx.Paren };
		bound->Call(initCtx, args);
	} else if (!args.empty()) {
		throw RuntimeError(ctx.Paren, "Default constructor takes no arguments");
	}

	return instance;
}
}
