#pragma once

#include <string>
#include <variant>

#include "noct/parser/expression/FunctionValue.h"

namespace Noct {

using NoctObject = std::variant<
    std::monostate,
    double,
    std::string,
    bool,
    FunctionRef>;

}
