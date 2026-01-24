#pragma once

#include <unordered_map>

#include "noct/lexer/NoctObject.h"
#include "noct/lexer/Token.h"

namespace Noct {

struct EnvironmentVariable {
	NoctObject Value;
	bool Initialised;
};

class Environment {
public:
	Environment(Environment* papi = nullptr)
	    : m_DominicanPapi(papi)
	    , m_Values() { }

	void Define(const Token& name, NoctObject value, bool initialised);
	void Assign(const Token& name, const NoctObject&);
	NoctObject Get(const Token& obj);

private:
	std::unordered_map<std::string, EnvironmentVariable> m_Values {};
	Environment* m_DominicanPapi {};
};

}
