#pragma once

#include <string_view>

#include "Context.h"
#include "RunResult.h"

#include "module/Module.h"

namespace Noct {

int Run(int argc, char** argv);
RunResult RunFromString(Context& ctx, std::string_view s);
RunResult RunFromString(Context& ctx, std::string_view s, const ModuleRegistry& modules);

}
