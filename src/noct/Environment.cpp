#include <fmt/format.h>

#include "noct/Environment.h"

#include "noct/exceptions/RuntimeException.h"

namespace Noct {

void Environment::Define(size_t slot, const NoctObject& value, bool initialised) {
	if (slot >= m_Values.size())
		throw RuntimeError(fmt::format("Internal error: define slot {} out of bounds (size={})", slot, m_Values.size()));

	m_Values[slot].Value = value;
	m_Values[slot].Initialised = initialised;
}

void Environment::Assign(size_t slot, size_t depth, const NoctObject& val) {
	if (slot >= m_Values.size()) {
		throw RuntimeError(fmt::format("Internal error: define slot {} out of bounds (size={})", slot, m_Values.size()));
	}

	if (!depth) {
		m_Values[slot].Value = val;
		m_Values[slot].Initialised = true;
		return;
	}

	if (!m_Papa)
		throw RuntimeError("invalid scope chain");

	m_Papa->Assign(slot, depth - 1, val);
}

NoctObject Environment::Get(size_t slot, size_t depth) const {
	if (slot >= m_Values.size()) {
		throw RuntimeError(fmt::format("Internal error: define slot {} out of bounds (size={})", slot, m_Values.size()));
	}

	if (!depth) {
		if (!m_Values[slot].Initialised) {
			throw RuntimeError("uninitialised variable");
		}

		return m_Values[slot].Value;
	}

	return m_Papa->Get(slot, depth - 1);
}

}
