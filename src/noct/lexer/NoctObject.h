#pragma once

#include <string>
#include <variant>

#include "noct/parser/expression/ClassInstanceFwd.h"
#include "noct/parser/expression/ClassValue.h"
#include "noct/parser/expression/FunctionValueFwd.h"

namespace Noct {

using NoctObject = std::variant<
    std::monostate,
    double,
    std::string,
    bool,
    FunctionValueRef,
    ClassValueRef,
    ClassInstanceRef>;

}
