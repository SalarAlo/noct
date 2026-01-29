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
	Environment(size_t frameSize, std::shared_ptr<Environment> papi = nullptr)
	    : m_Papa(papi)
	    , m_Values(frameSize, EnvironmentVariable {}) {
	}

	void Define(size_t slot, const NoctObject& value, bool initialised);
	void Assign(size_t slot, size_t depth, const NoctObject& val);
	NoctObject Get(size_t slot, size_t depth) const;

private:
	std::vector<EnvironmentVariable> m_Values {};
	std::shared_ptr<Environment> m_Papa {};
};

}
