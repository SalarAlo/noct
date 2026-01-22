#pragma once

#include <unordered_map>

#include "noct/lexer/Token.h"

namespace Noct {

struct EnvironmentVariable {
	NoctLiteral Value;
	bool Initialised;
};

class Environment {
public:
	Environment(Environment* papi = nullptr)
	    : m_DominicanPapi(papi)
	    , m_Values() { }

	void Define(const Token& name, const NoctLiteral& value, bool initialised);
	void Assign(const Token& name, const NoctLiteral&);
	NoctLiteral Get(const Token& obj);

private:
	std::unordered_map<std::string, EnvironmentVariable> m_Values {};
	Environment* m_DominicanPapi {};
};

}
