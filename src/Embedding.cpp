#include "Embedding.h"

namespace Noct {

bool CompileResult::HasErrors() const {
	for (const auto& diagnostic : m_Diagnostics) {
		if (diagnostic.Severity == NoctSeverity::Error)
			return true;
	}
	return false;
}

bool DeclarationResult::HasErrors() const {
	for (const auto& diagnostic : m_Diagnostics) {
		if (diagnostic.Severity == NoctSeverity::Error)
			return true;
	}
	return false;
}

}
