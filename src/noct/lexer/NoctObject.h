#pragma once

#include <string>
#include <variant>

#include "noct/parser/expression/ClassInstanceFwd.h"
#include "noct/parser/expression/ICallableFwd.h"

namespace Noct {

using NoctObject = std::variant<
    std::monostate,
    double,
    std::string,
    bool,
    ClassInstanceRef,
    CallableRef>;

static_assert(std::variant_size_v<NoctObject> == 6, "NoctObject alternative count changed");

static_assert(std::is_same_v<std::variant_alternative_t<0, NoctObject>, std::monostate>);
static_assert(std::is_same_v<std::variant_alternative_t<1, NoctObject>, double>);
static_assert(std::is_same_v<std::variant_alternative_t<2, NoctObject>, std::string>);
static_assert(std::is_same_v<std::variant_alternative_t<3, NoctObject>, bool>);
static_assert(std::is_same_v<std::variant_alternative_t<4, NoctObject>, ClassInstanceRef>);
static_assert(std::is_same_v<std::variant_alternative_t<5, NoctObject>, CallableRef>);

}
