#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

#include "Diagnostic.h"

namespace Noct {

struct ScriptId {
	std::string Value {};

	friend bool operator==(const ScriptId&, const ScriptId&) = default;
	explicit operator bool() const { return !Value.empty(); }
};

struct RuntimeConfig {
	std::filesystem::path ProjectRoot {};
};

enum class NoctFieldType {
	Int,
	Float,
	Bool,
	String
};

using NoctValue = std::variant<std::monostate, int64_t, double, bool, std::string>;

struct NoctFieldDeclaration {
	std::string Name;
	NoctFieldType Type {};
	NoctValue DefaultValue {};
	NoctSourceLocation Location {};
};

struct NoctComponentDeclaration {
	std::string Name;
	std::vector<NoctFieldDeclaration> Fields;
	ScriptId SourceScript {};
	NoctSourceLocation Location {};
};

class CompiledScript {
public:
	CompiledScript() = default;
	CompiledScript(ScriptId id, std::filesystem::path path, std::string source, std::vector<std::string> imports)
	    : m_Id(std::move(id))
	    , m_Path(std::move(path))
	    , m_Source(std::move(source))
	    , m_Imports(std::move(imports)) { }

	const ScriptId& Id() const { return m_Id; }
	const std::filesystem::path& Path() const { return m_Path; }
	std::string_view Source() const { return m_Source; }
	const std::vector<std::string>& Imports() const { return m_Imports; }

private:
	ScriptId m_Id {};
	std::filesystem::path m_Path {};
	std::string m_Source {};
	std::vector<std::string> m_Imports {};
};

class CompileResult {
public:
	static CompileResult Success(CompiledScript script, std::vector<NoctDiagnostic> diagnostics = {}) {
		CompileResult result;
		result.m_Script = std::move(script);
		result.m_Diagnostics = std::move(diagnostics);
		return result;
	}

	static CompileResult Failure(std::vector<NoctDiagnostic> diagnostics) {
		CompileResult result;
		result.m_Diagnostics = std::move(diagnostics);
		return result;
	}

	explicit operator bool() const { return m_Script.has_value() && !HasErrors(); }
	bool HasErrors() const;
	const std::vector<NoctDiagnostic>& diagnostics() const { return m_Diagnostics; }
	const std::vector<NoctDiagnostic>& Diagnostics() const { return m_Diagnostics; }
	const CompiledScript& script() const { return *m_Script; }
	const CompiledScript& Script() const { return *m_Script; }

private:
	std::optional<CompiledScript> m_Script;
	std::vector<NoctDiagnostic> m_Diagnostics;
};

class DeclarationResult {
public:
	static DeclarationResult Success(std::vector<NoctComponentDeclaration> components, std::vector<NoctDiagnostic> diagnostics = {}) {
		DeclarationResult result;
		result.m_Components = std::move(components);
		result.m_Diagnostics = std::move(diagnostics);
		return result;
	}

	static DeclarationResult Failure(std::vector<NoctDiagnostic> diagnostics) {
		DeclarationResult result;
		result.m_Diagnostics = std::move(diagnostics);
		return result;
	}

	explicit operator bool() const { return !HasErrors(); }
	bool HasErrors() const;
	const std::vector<NoctDiagnostic>& diagnostics() const { return m_Diagnostics; }
	const std::vector<NoctDiagnostic>& Diagnostics() const { return m_Diagnostics; }
	const std::vector<NoctComponentDeclaration>& components() const { return m_Components; }
	const std::vector<NoctComponentDeclaration>& Components() const { return m_Components; }

private:
	std::vector<NoctComponentDeclaration> m_Components;
	std::vector<NoctDiagnostic> m_Diagnostics;
};

struct ModuleDependencyGraph {
	std::unordered_map<std::string, std::vector<std::string>> ImportsByScript;
	std::unordered_map<std::string, std::vector<std::string>> ScriptsByModule;
};

}
