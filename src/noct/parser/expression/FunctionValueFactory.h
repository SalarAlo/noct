#pragma once

#include "noct/Environment.h"

#include "noct/parser/expression/FunctionValue.h"

#include "noct/parser/expression/variants/Lambda.h"

#include "noct/parser/statement/FunctionDecleration.h"

namespace Noct {
class FunctionValueFactory {
public:
	FunctionValueFactory(const std::shared_ptr<Environment>& environment)
	    : m_Env(environment) { };

	FunctionValueRef operator()(const Lambda& lambda) {
		return std::make_shared<FunctionValue>(
		    &lambda.Body,
		    lambda.Parameters,
		    std::nullopt,
		    lambda.FrameSize,
		    0,
		    m_Env);
	}

	FunctionValueRef operator()(const FunctionDecleration& functionDecl) {
		return std::make_shared<FunctionValue>(
		    &functionDecl.Body,
		    functionDecl.Parameters,
		    std::nullopt,
		    functionDecl.FrameSize,
		    functionDecl.ClosureSize,
		    m_Env);
	}

private:
	std::shared_ptr<Environment> m_Env;
};

}
