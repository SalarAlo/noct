#include "noct/parser/expression/FunctionValue.h"

#include "noct/parser/expression/ClassInstance.h"

namespace Noct {

FunctionValue::~FunctionValue() = default;

FunctionValueRef FunctionValue::Bind(const ClassInstanceRef& instance) {
	const size_t size = std::max<size_t>(ClosureSize, 1);
	auto methodClosure = std::make_shared<Environment>(size, Closure);
	methodClosure->Define(0, instance, true);

	return std::make_shared<FunctionValue>(
	    Body,
	    ParameterNames,
	    Name,
	    FrameSize,
	    ClosureSize,
	    methodClosure);
}

}
