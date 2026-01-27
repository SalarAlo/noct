#pragma once

#include <string_view>

#include "noct/Context.h"
#include "noct/RunResult.h"

namespace Noct {

int Run(int argc, char** argv);
RunResult RunFromString(Context& ctx, std::string_view s);

}
