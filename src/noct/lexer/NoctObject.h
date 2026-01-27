#pragma once

#include <memory>
#include <string>
#include <variant>

#include "noct/parser/expression/ClassInstance.h"
#include "noct/parser/expression/ClassValue.h"

namespace Noct {
struct FunctionValue;
using FunctionValueRef = std::shared_ptr<FunctionValue>;

using NoctObject = std::variant<
    std::monostate,
    double,
    std::string,
    bool,
    FunctionValueRef,
    ClassValueRef,
    ClassInstance>;

}
