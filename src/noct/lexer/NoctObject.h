#pragma once

#include <memory>
#include <string>
#include <variant>

namespace Noct {
struct FunctionValue;
using FunctionRef = std::shared_ptr<FunctionValue>;

using NoctObject = std::variant<
    std::monostate,
    double,
    std::string,
    bool,
    FunctionRef>;

}
