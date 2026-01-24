#include "noct/Environment.h"

#include <fmt/format.h>

#include "noct/exceptions/RuntimeException.h"
#include "noct/lexer/Token.h"

namespace Noct {

void Environment::Define(const Token& name, NoctObject obj, bool initialised) {
	if (m_Values.contains(name.Lexeme)) {
		throw RuntimeError(name, fmt::format("Redefining variable '{}'.", name.Lexeme));
	}
	m_Values.emplace(name.Lexeme, EnvironmentVariable { obj, initialised });
}

void Environment::Assign(const Token& name, const NoctObject& val) {
	auto it = m_Values.find(name.Lexeme);

	if (it != m_Values.end()) {
		it->second.Value = val;
		it->second.Initialised = true;
		return;
	}

	if (m_DominicanPapi) {
		m_DominicanPapi->Assign(name, val);
		return;
	}

	throw RuntimeError(name, fmt::format("Undefined variable '{}'.", name.Lexeme));
}

NoctObject Environment::Get(const Token& name) {
	auto it = m_Values.find(name.Lexeme);

	if (it != m_Values.end()) {
		if (!it->second.Initialised) {
			throw RuntimeError(name, fmt::format("Uninitialised variable '{}'.", name.Lexeme));
		}

		return it->second.Value;
	}

	if (m_DominicanPapi) {
		return m_DominicanPapi->Get(name);
	}

	throw RuntimeError(name, fmt::format("Undefined variable '{}'.", name.Lexeme));
}

}
