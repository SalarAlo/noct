#pragma once

#include <memory>
#include <string>

namespace Noct {

struct ClassValue {
	std::string Name;
};

using ClassValueRef = std::shared_ptr<ClassValue>;

}
