#pragma once

#include <string>
#include <variant>

#include "parser/expression/ClassInstanceFwd.h"
#include "parser/expression/ICallableFwd.h"
#include "parser/expression/ModuleFwd.h"

namespace Noct {

using NoctObject = std::variant<
    std::monostate,
    double,
    std::string,
    bool,
    ClassInstanceRef,
    ModuleRef,
    CallableRef>;

}
