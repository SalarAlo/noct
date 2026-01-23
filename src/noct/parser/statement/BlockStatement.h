#pragma once

#include <vector>

#include "noct/parser/statement/StatementFwd.h"

namespace Noct {

struct BlockStatement {
	std::vector<StatementPtr> Statements;
};

}
