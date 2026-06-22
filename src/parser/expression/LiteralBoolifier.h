#pragma once

#include <string>
#include <variant>

#include "parser/expression/ClassInstanceFwd.h"
#include "parser/expression/ICallableFwd.h"
#include "parser/expression/ModuleFwd.h"

namespace Noct {
struct LiteralBoolifier {
	bool operator()(const double& value) const {
		return value != 0;
	}

	bool operator()(const std::string& value) const {
		return !value.empty();
	}

	bool operator()(const bool& value) const {
		return value;
	}

	bool operator()(const std::monostate&) const {
		return false;
	}

	bool operator()(const CallableRef&) const {
		return true;
	}

	bool operator()(const ModuleRef&) const {
		return true;
	}

	bool operator()(const ClassInstanceRef&) const {
		return true;
	}
};
}
