#include "noct/Environment.h"
#include "noct/exceptions/RuntimeException.h"

#include <fmt/format.h>

namespace Noct {

void Environment::Define(std::string_view name, const NoctLiteral& obj) {
	m_Values[name.data()] = obj;
}

void Environment::Assign(const Token& name, const NoctLiteral& val) {
	if (m_Values.contains(name.Lexeme)) {
		m_Values[name.Lexeme] = val;
		return;
	}

	throw RuntimeError(name, fmt::format("Undefined variable '{}'.", name.Lexeme));
}

NoctLiteral Environment::Get(const Token& name) {
	if (m_Values.contains(name.Lexeme)) {
		return m_Values[name.Lexeme];
	}
	throw RuntimeError(name, fmt::format("Undefined variable '{}'.", name.Lexeme));
}

}
