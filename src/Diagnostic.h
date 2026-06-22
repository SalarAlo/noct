#pragma once

#include <filesystem>
#include <string>
#include <utility>
#include <vector>

namespace Noct {

enum class NoctSeverity {
	Info,
	Warning,
	Error
};

struct NoctSourceLocation {
	std::filesystem::path File {};
	int Line {};
	int Column {};
};

struct NoctDiagnostic {
	NoctSeverity Severity { NoctSeverity::Error };
	NoctSourceLocation Location {};
	std::string Message {};
	std::string Code {};
};

class DiagnosticBag {
public:
	void Add(NoctSeverity severity, NoctSourceLocation location, std::string message, std::string code = {}) {
		m_Diagnostics.push_back({
		    .Severity = severity,
		    .Location = std::move(location),
		    .Message = std::move(message),
		    .Code = std::move(code),
		});
	}

	bool HasErrors() const {
		for (const auto& diagnostic : m_Diagnostics) {
			if (diagnostic.Severity == NoctSeverity::Error)
				return true;
		}
		return false;
	}

	const std::vector<NoctDiagnostic>& All() const { return m_Diagnostics; }
	std::vector<NoctDiagnostic>& All() { return m_Diagnostics; }
	void Clear() { m_Diagnostics.clear(); }

private:
	std::vector<NoctDiagnostic> m_Diagnostics;
};

}
