#pragma once

#include <variant>

#include "parser/statement/BlockStatement.h"
#include "parser/statement/BreakStatement.h"
#include "parser/statement/ClassDecleration.h"
#include "parser/statement/ExpressionStatement.h"
#include "parser/statement/FunctionDecleration.h"
#include "parser/statement/IfStatement.h"
#include "parser/statement/ImportStatement.h"
#include "parser/statement/PrintStatement.h"
#include "parser/statement/ReturnStatement.h"
#include "parser/statement/VariableDecleration.h"
#include "parser/statement/WhileStatement.h"

namespace Noct {

struct Statement {
public:
	using Variant = std::variant<
	    BlockStatement, ExpressionStatement, IfStatement,
	    VariableDecleration, FunctionDecleration, ClassDecleration,
	    ImportStatement, PrintStatement, WhileStatement,
	    BreakStatement, ReturnStatement>;

	Statement(const Statement&) = delete;
	Statement& operator=(const Statement&) = delete;

	Statement(Statement&&) noexcept = default;
	Statement& operator=(Statement&&) noexcept = default;

public:
	Variant Instruction;

	template <class T>
	explicit Statement(T&& v)
	    : Instruction(std::forward<T>(v)) { }
};

}
