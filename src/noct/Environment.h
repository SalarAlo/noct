#pragma once

#include <vector>

#include "noct/lexer/NoctObject.h"

namespace Noct {

struct EnvironmentVariable {
	NoctObject Value { std::monostate {} };
	bool Initialised { false };
};

class Environment {
public:
	Environment(size_t size, Environment* papi = nullptr)
	    : m_DominicanPapi(papi)
	    , m_Values(size, EnvironmentVariable {}) {
	}

	void Define(size_t slot, const NoctObject& value, bool initialised);
	void Assign(size_t slot, size_t depth, const NoctObject& val);
	NoctObject Get(size_t slot, size_t depth) const;

private:
	std::vector<EnvironmentVariable> m_Values {};
	Environment* m_DominicanPapi {};
};

}
