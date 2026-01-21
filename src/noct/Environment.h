#pragma once

#include <unordered_map>

#include "noct/lexer/Token.h"

namespace Noct {

class Environment {
public:
	void Define(std::string_view, const NoctLiteral&);
	void Assign(const Token& name, const NoctLiteral&);
	NoctLiteral Get(const Token& obj);

private:
	std::unordered_map<std::string, NoctLiteral> m_Values {};
};

}
