#pragma once
#include <memory>
#include <utility>

#include "parser/statement/Statement.h"
#include "parser/statement/StatementFwd.h"

namespace Noct {

template <typename T, typename... Args>
StatementPtr make_statement(Args&&... args) {
	return std::make_unique<Statement>(T { std::forward<Args>(args)... });
}

}
