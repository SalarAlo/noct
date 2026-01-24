#pragma once

#include "noct/parser/statement/BlockStatement.h"
#include "noct/parser/statement/ExpressionStatement.h"
#include "noct/parser/statement/FunctionDecleration.h"
#include "noct/parser/statement/IfStatement.h"
#include "noct/parser/statement/PrintStatement.h"
#include "noct/parser/statement/VariableDecleration.h"
#include "noct/parser/statement/WhileStatement.h"
#include "noct/parser/statement/BreakStatement.h"

namespace Noct {

struct Statement {
public:
	using Variant = std::variant<
	    BlockStatement, ExpressionStatement, IfStatement,
	    VariableDecleration, FunctionDecleration,
	    PrintStatement, WhileStatement, BreakStatement>;

public:
	Variant Instruction;

	template <class T>
	explicit Statement(T&& v)
	    : Instruction(std::forward<T>(v)) { }
};

}
