#include "Runtime.h"

#include <fstream>
#include <sstream>
#include <unordered_set>

#include <fmt/format.h>

#include "Environment.h"
#include "Run.h"
#include "interpreter/Interpreter.h"
#include "lexer/Lexer.h"
#include "parser/Parser.h"
#include "resolver/Resolver.h"

namespace Noct {

namespace {
	std::string ReadFile(const std::filesystem::path& path) {
		std::ifstream file { path };
		if (!file.is_open()) {
			throw std::runtime_error(fmt::format("Unable to open script '{}'", path.string()));
		}

		std::ostringstream contents;
		contents << file.rdbuf();
		return contents.str();
	}

	NoctSourceLocation RuntimeLocation(const std::filesystem::path& path) {
		return {
		    .File = path,
		    .Line = 0,
		    .Column = 0,
		};
	}

	std::optional<NoctFieldType> ParseFieldType(std::string_view type) {
		if (type == "Int")
			return NoctFieldType::Int;
		if (type == "Float")
			return NoctFieldType::Float;
		if (type == "Bool")
			return NoctFieldType::Bool;
		if (type == "String")
			return NoctFieldType::String;
		return {};
	}

	bool DefaultMatchesType(NoctFieldType type, const NoctValue& value) {
		switch (type) {
		case NoctFieldType::Int:
			return std::holds_alternative<int64_t>(value);
		case NoctFieldType::Float:
			return std::holds_alternative<double>(value) || std::holds_alternative<int64_t>(value);
		case NoctFieldType::Bool:
			return std::holds_alternative<bool>(value);
		case NoctFieldType::String:
			return std::holds_alternative<std::string>(value);
		}
		return false;
	}
}

Runtime::Runtime(RuntimeConfig config)
    : m_Config(std::move(config)) {
	initialize();
}

Runtime::~Runtime() {
	shutdown();
}

void Runtime::initialize() {
	m_Initialized = true;
}

void Runtime::shutdown() {
	invalidate_all();
	m_Initialized = false;
}

void Runtime::reset() {
	invalidate_all();
}

Module& Runtime::registerModule(std::string name) {
	return m_Modules.RegisterModule(std::move(name));
}

RunResult Runtime::RunString(Context& ctx, std::string_view source) {
	return RunFromString(ctx, source, m_Modules);
}

void Runtime::add_module_search_path(std::filesystem::path path) {
	m_ModuleSearchPaths.push_back(std::move(path));
}

void Runtime::set_project_root(std::filesystem::path path) {
	m_Config.ProjectRoot = std::move(path);
}

CompileResult Runtime::compile_source(ScriptId id, std::filesystem::path sourcePath, std::string_view source) {
	Context context {};
	context.LoggingEnabled = false;
	context.SourcePath = sourcePath;

	Lexer lexer { source, context };
	Parser parser { lexer.ScanTokens(), context };
	auto statements = parser.parse();

	if (!context.HadParseError) {
		Resolver resolver { context };
		resolver.Resolve(statements);
	}

	std::vector<NoctDiagnostic> diagnostics = context.Diagnostics;
	std::vector<std::string> imports = ExtractImports(source, sourcePath, diagnostics);

	if (context.HadParseError || context.HadSemanticError) {
		return CompileResult::Failure(std::move(diagnostics));
	}

	for (const auto& import : imports) {
		if (m_Modules.Find(import))
			continue;
		if (auto modulePath = ResolveModulePath(import, sourcePath)) {
			try {
				const std::string moduleSource = ReadFile(*modulePath);
				std::vector<NoctDiagnostic> moduleDiagnostics;
				auto moduleImports = ExtractImports(moduleSource, *modulePath, moduleDiagnostics);
				diagnostics.insert(diagnostics.end(), moduleDiagnostics.begin(), moduleDiagnostics.end());

				for (const auto& nestedImport : moduleImports) {
					if (nestedImport == import) {
						diagnostics.push_back({
						    .Severity = NoctSeverity::Error,
						    .Location = RuntimeLocation(*modulePath),
						    .Message = fmt::format("Cyclic import detected for module '{}'", import),
						    .Code = "cyclic-import",
						});
					}
				}

				CompiledScript moduleScript {
					ScriptId { import },
					*modulePath,
					moduleSource,
					std::move(moduleImports),
				};
				UpdateDependencyGraph(moduleScript);
				m_ModuleCache[import] = std::move(moduleScript);
			} catch (const std::exception& e) {
				diagnostics.push_back({
				    .Severity = NoctSeverity::Error,
				    .Location = RuntimeLocation(*modulePath),
				    .Message = e.what(),
				    .Code = "module-import",
				});
			}
			continue;
		}

		diagnostics.push_back({
		    .Severity = NoctSeverity::Error,
		    .Location = RuntimeLocation(sourcePath),
		    .Message = fmt::format("Unknown module '{}'", import),
		    .Code = "module-import",
		});
	}

	if (!diagnostics.empty()) {
		for (const auto& diagnostic : diagnostics) {
			if (diagnostic.Severity == NoctSeverity::Error)
				return CompileResult::Failure(std::move(diagnostics));
		}
	}

	CompiledScript compiled { std::move(id), std::move(sourcePath), std::string { source }, std::move(imports) };
	UpdateDependencyGraph(compiled);
	if (compiled.Id())
		m_ScriptsById[compiled.Id().Value] = compiled;

	return CompileResult::Success(std::move(compiled), std::move(diagnostics));
}

CompileResult Runtime::compile_file(ScriptId id, std::filesystem::path sourcePath) {
	try {
		return compile_source(std::move(id), sourcePath, ReadFile(sourcePath));
	} catch (const std::exception& e) {
		return CompileResult::Failure({
		    {
		        .Severity = NoctSeverity::Error,
		        .Location = RuntimeLocation(sourcePath),
		        .Message = e.what(),
		        .Code = "file-load",
		    },
		});
	}
}

DeclarationResult Runtime::evaluate_declarations(const CompiledScript& script) {
	return extract_component_declarations(script);
}

DeclarationResult Runtime::extract_component_declarations(const CompiledScript& script) {
	ModuleRegistry modules = m_Modules;
	std::vector<NoctComponentDeclaration> components;
	std::vector<NoctDiagnostic> diagnostics;

	modules.RegisterModule("Component")
	    .context_function("define", [&](NativeCallContext& ctx) -> NoctObject {
		    if (ctx.argument_count() < 1) {
			    ctx.add_error("Component.define requires a component name.", "component-declaration");
			    return std::monostate {};
		    }

		    NoctComponentDeclaration component {
		        .Name = ctx.require_string(0),
		        .SourceScript = script.Id(),
		        .Location = ctx.location(),
		    };

		    const size_t remaining = ctx.argument_count() - 1;
		    if (remaining % 3 != 0) {
			    ctx.add_error("Component.define fields must be triples: field_name, field_type, default_value.", "component-declaration");
			    return std::monostate {};
		    }

		    for (size_t i = 1; i < ctx.argument_count(); i += 3) {
			    std::string fieldName = ctx.require_string(i);
			    std::string fieldTypeName = ctx.require_string(i + 1);
			    std::optional<NoctFieldType> fieldType = ParseFieldType(fieldTypeName);
			    if (!fieldType) {
				    ctx.add_error(fmt::format("Unknown component field type '{}'.", fieldTypeName), "component-field-type");
				    return std::monostate {};
			    }

			    NoctValue defaultValue = ctx.require_value(i + 2);
			    if (*fieldType == NoctFieldType::Int) {
				    if (const auto* numeric = std::get_if<double>(&defaultValue)) {
					    const auto asInt = static_cast<int64_t>(*numeric);
					    if (static_cast<double>(asInt) == *numeric)
						    defaultValue = asInt;
				    }
			    }

			    if (!DefaultMatchesType(*fieldType, defaultValue)) {
				    ctx.add_error(fmt::format("Default value for field '{}' does not match declared type.", fieldName), "component-default-type");
				    return std::monostate {};
			    }

			    component.Fields.push_back({
			        .Name = std::move(fieldName),
			        .Type = *fieldType,
			        .DefaultValue = std::move(defaultValue),
			        .Location = ctx.location(),
			    });
		    }

		    ctx.add_component(std::move(component));
		    return std::monostate {};
	    });

	Context context {};
	context.LoggingEnabled = false;
	context.SourcePath = script.Path();

	Lexer lexer { script.Source(), context };
	Parser parser { lexer.ScanTokens(), context };
	auto statements = parser.parse();
	if (!context.HadParseError) {
		Resolver resolver { context };
		resolver.Resolve(statements);
		if (!context.HadSemanticError) {
			Interpreter interpreter {
			    context,
			    modules,
			    script.Id(),
			    script.Path(),
			    &components,
			    &diagnostics,
			};
			interpreter.SetGlobalEnvironment(std::make_shared<Environment>(resolver.GetGlobalFrameSize()));
			interpreter.Interpret(statements);
		}
	}

	diagnostics.insert(diagnostics.end(), context.Diagnostics.begin(), context.Diagnostics.end());
	for (const auto& diagnostic : diagnostics) {
		if (diagnostic.Severity == NoctSeverity::Error)
			return DeclarationResult::Failure(std::move(diagnostics));
	}

	return DeclarationResult::Success(std::move(components), std::move(diagnostics));
}

void Runtime::invalidate_script(const ScriptId& id) {
	if (!id)
		return;

	m_ScriptsById.erase(id.Value);
	m_DependencyGraph.ImportsByScript.erase(id.Value);
	for (auto& [_, scripts] : m_DependencyGraph.ScriptsByModule) {
		std::erase(scripts, id.Value);
	}
}

void Runtime::invalidate_module(std::string_view moduleName) {
	m_ModuleCache.erase(std::string { moduleName });
	m_DependencyGraph.ScriptsByModule.erase(std::string { moduleName });
}

void Runtime::invalidate_all() {
	m_ScriptsById.clear();
	m_ModuleCache.clear();
	m_DependencyGraph = {};
}

std::vector<std::string> Runtime::scripts_depending_on(std::string_view moduleName) const {
	if (auto iter = m_DependencyGraph.ScriptsByModule.find(std::string { moduleName }); iter != m_DependencyGraph.ScriptsByModule.end()) {
		return iter->second;
	}
	return {};
}

std::vector<std::string> Runtime::ExtractImports(std::string_view source, const std::filesystem::path& path, std::vector<NoctDiagnostic>& diagnostics) const {
	Context context {};
	context.LoggingEnabled = false;
	context.SourcePath = path;
	Lexer lexer { source, context };
	const auto& tokens = lexer.ScanTokens();

	std::vector<std::string> imports;
	for (size_t i = 0; i + 1 < tokens.size(); ++i) {
		if (tokens[i].Type != TokenType::Import)
			continue;
		if (tokens[i + 1].Type != TokenType::Identifier) {
			diagnostics.push_back({
			    .Severity = NoctSeverity::Error,
			    .Location = {
			        .File = path,
			        .Line = static_cast<int>(tokens[i].Line),
			        .Column = 0,
			    },
			    .Message = "Expected module name after import.",
			    .Code = "module-import",
			});
			continue;
		}
		imports.push_back(tokens[i + 1].Lexeme);
	}

	return imports;
}

std::optional<std::filesystem::path> Runtime::ResolveModulePath(std::string_view moduleName, const std::filesystem::path& currentScript) const {
	const auto modulePath = std::filesystem::path { std::string { moduleName } + ".noct" };
	std::vector<std::filesystem::path> roots;
	if (!currentScript.empty() && currentScript.has_parent_path())
		roots.push_back(currentScript.parent_path());
	if (!m_Config.ProjectRoot.empty())
		roots.push_back(m_Config.ProjectRoot);
	roots.insert(roots.end(), m_ModuleSearchPaths.begin(), m_ModuleSearchPaths.end());

	for (const auto& root : roots) {
		auto candidate = root / modulePath;
		if (std::filesystem::exists(candidate))
			return candidate;
	}
	return {};
}

void Runtime::UpdateDependencyGraph(const CompiledScript& script) {
	const std::string scriptKey = script.Id() ? script.Id().Value : script.Path().string();
	m_DependencyGraph.ImportsByScript[scriptKey] = script.Imports();
	for (const auto& import : script.Imports()) {
		auto& scripts = m_DependencyGraph.ScriptsByModule[import];
		if (std::find(scripts.begin(), scripts.end(), scriptKey) == scripts.end())
			scripts.push_back(scriptKey);
	}
}

}
