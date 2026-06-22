
#pragma once

#include <memory>

namespace Noct {

struct CallContext;
struct ICallable;
using CallableRef = std::shared_ptr<ICallable>;

}
