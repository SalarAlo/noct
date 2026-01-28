#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "noct/parser/expression/FunctionValueFwd.h"

namespace Noct {

struct ClassValue {
	std::string Name;
	std::unordered_map<std::string, FunctionValueRef> Methods {};
};

using ClassValueRef = std::shared_ptr<ClassValue>;

}
