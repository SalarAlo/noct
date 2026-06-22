#pragma once

#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "Context.h"
#include "Embedding.h"
#include "RunResult.h"
#include "Version.h"

#include "module/Module.h"

namespace Noct {

class Runtime {
public:
	explicit Runtime(RuntimeConfig config = {});
	~Runtime();

	void initialize();
	void Initialize() { initialize(); }
	void shutdown();
	void Shutdown() { shutdown(); }
	void reset();
	void Reset() { reset(); }

	Module& registerModule(std::string name);
	Module& RegisterModule(std::string name) { return registerModule(std::move(name)); }
	Module& register_native_module(std::string name) { return registerModule(std::move(name)); }
	Module& RegisterNativeModule(std::string name) { return registerModule(std::move(name)); }

	RunResult RunString(Context& ctx, std::string_view source);

	void add_module_search_path(std::filesystem::path path);
	void AddModuleSearchPath(std::filesystem::path path) { add_module_search_path(std::move(path)); }
	void set_project_root(std::filesystem::path path);
	void SetProjectRoot(std::filesystem::path path) { set_project_root(std::move(path)); }

	CompileResult compile_source(ScriptId id, std::filesystem::path sourcePath, std::string_view source);
	CompileResult CompileSource(ScriptId id, std::filesystem::path sourcePath, std::string_view source) {
		return compile_source(std::move(id), std::move(sourcePath), source);
	}

	CompileResult compile_file(ScriptId id, std::filesystem::path sourcePath);
	CompileResult CompileFile(ScriptId id, std::filesystem::path sourcePath) {
		return compile_file(std::move(id), std::move(sourcePath));
	}

	DeclarationResult evaluate_declarations(const CompiledScript& script);
	DeclarationResult EvaluateDeclarations(const CompiledScript& script) { return evaluate_declarations(script); }
	DeclarationResult extract_component_declarations(const CompiledScript& script);
	DeclarationResult ExtractComponentDeclarations(const CompiledScript& script) { return extract_component_declarations(script); }

	void invalidate_script(const ScriptId& id);
	void InvalidateScript(const ScriptId& id) { invalidate_script(id); }
	void invalidate_module(std::string_view moduleName);
	void InvalidateModule(std::string_view moduleName) { invalidate_module(moduleName); }
	void invalidate_all();
	void InvalidateAll() { invalidate_all(); }

	ModuleDependencyGraph module_dependency_graph() const { return m_DependencyGraph; }
	Noct::ModuleDependencyGraph GetModuleDependencyGraph() const { return module_dependency_graph(); }
	std::vector<std::string> scripts_depending_on(std::string_view moduleName) const;
	std::vector<std::string> ScriptsDependingOn(std::string_view moduleName) const { return scripts_depending_on(moduleName); }

	const ModuleRegistry& Modules() const { return m_Modules; }

private:
	std::vector<std::string> ExtractImports(std::string_view source, const std::filesystem::path& path, std::vector<NoctDiagnostic>& diagnostics) const;
	std::optional<std::filesystem::path> ResolveModulePath(std::string_view moduleName, const std::filesystem::path& currentScript) const;
	void UpdateDependencyGraph(const CompiledScript& script);

private:
	RuntimeConfig m_Config;
	ModuleRegistry m_Modules;
	std::vector<std::filesystem::path> m_ModuleSearchPaths;
	std::unordered_map<std::string, CompiledScript> m_ScriptsById;
	std::unordered_map<std::string, CompiledScript> m_ModuleCache;
	Noct::ModuleDependencyGraph m_DependencyGraph;
	bool m_Initialized {};
};

}

namespace noct = Noct;
