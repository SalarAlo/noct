#pragma once

#include <memory>
#include <vector>

namespace Noct {

struct Statement;
using StatementPtr = std::unique_ptr<Statement>;

using StatementVector = std::vector<Statement>;
using StatementPtrVector = std::vector<StatementPtr>;

}
