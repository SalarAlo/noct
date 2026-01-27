#include <fmt/format.h>

#include "noct/Environment.h"

#include "noct/exceptions/RuntimeException.h"

namespace Noct {

void Environment::Define(size_t slot, const NoctObject& value, bool initialised) {
	m_Values[slot].Value = value;
	m_Values[slot].Initialised = initialised;
}

void Environment::Assign(size_t slot, size_t depth, const NoctObject& val) {
	if (!depth) {
		m_Values[slot].Value = val;
		m_Values[slot].Initialised = true;
		return;
	}

	if (!m_DominicanPapi)
		throw RuntimeError("invalid scope chain");

	m_DominicanPapi->Assign(slot, depth - 1, val);
}

NoctObject Environment::Get(size_t slot, size_t depth) const {
	if (!depth) {
		if (!m_Values[slot].Initialised) {
			throw RuntimeError("uninitialised variable");
		}

		return m_Values[slot].Value;
	}

	return m_DominicanPapi->Get(slot, depth - 1);
}

}
