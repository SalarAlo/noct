#pragma once

#include "noct/parser/expression/FunctionValue.h"

#include <string>
#include <variant>

namespace Noct {

using NoctObject = std::variant<
    std::monostate,
    double,
    std::string,
    bool,
    std::shared_ptr<FunctionValue>>;

}
