#include "module/Module.h"

#include "lexer/TokenType.h"

namespace Noct {

namespace {
	NoctValue ToPublicValue(const NoctObject& value) {
		if (const auto* number = std::get_if<double>(&value))
			return *number;
		if (const auto* string = std::get_if<std::string>(&value))
			return *string;
		if (const auto* boolean = std::get_if<bool>(&value))
			return *boolean;
		if (std::holds_alternative<std::monostate>(value))
			return std::monostate {};

		throw RuntimeError("Expected scalar declaration value.");
	}
}

const NoctObject& NativeCallContext::argument(size_t index) const {
	return RequireArgument(index);
}

std::string NativeCallContext::require_string(size_t index) {
	const auto& value = RequireArgument(index);
	if (const auto* string = std::get_if<std::string>(&value))
		return *string;

	throw RuntimeError(location().Line == 0 ? Token(TokenType::Eof, "", 0) : m_CallContext.Paren,
	    fmt::format("Argument {} to '{}' must be a string.", index + 1, m_FunctionName));
}

double NativeCallContext::require_number(size_t index) {
	const auto& value = RequireArgument(index);
	if (const auto* number = std::get_if<double>(&value))
		return *number;

	throw RuntimeError(m_CallContext.Paren,
	    fmt::format("Argument {} to '{}' must be a number.", index + 1, m_FunctionName));
}

bool NativeCallContext::require_bool(size_t index) {
	const auto& value = RequireArgument(index);
	if (const auto* boolean = std::get_if<bool>(&value))
		return *boolean;

	throw RuntimeError(m_CallContext.Paren,
	    fmt::format("Argument {} to '{}' must be a bool.", index + 1, m_FunctionName));
}

NoctValue NativeCallContext::require_value(size_t index) {
	try {
		return ToPublicValue(RequireArgument(index));
	} catch (const RuntimeError&) {
		throw RuntimeError(m_CallContext.Paren,
		    fmt::format("Argument {} to '{}' must be a scalar declaration value.", index + 1, m_FunctionName));
	}
}

void NativeCallContext::add_error(std::string message, std::string code) {
	if (!m_CallContext.Diagnostics)
		return;

	m_CallContext.Diagnostics->push_back({
	    .Severity = NoctSeverity::Error,
	    .Location = location(),
	    .Message = std::move(message),
	    .Code = std::move(code),
	});
}

void NativeCallContext::add_component(NoctComponentDeclaration component) {
	if (!m_CallContext.Components)
		return;

	if (!component.SourceScript)
		component.SourceScript = m_CallContext.CurrentScript;
	if (component.Location.Line == 0)
		component.Location = location();

	m_CallContext.Components->push_back(std::move(component));
}

NoctSourceLocation NativeCallContext::location() const {
	return {
	    .File = m_CallContext.CurrentFile,
	    .Line = static_cast<int>(m_CallContext.Paren.Line),
	    .Column = 0,
	};
}

const NoctObject& NativeCallContext::RequireArgument(size_t index) const {
	if (index < m_Args.size())
		return m_Args[index];

	throw RuntimeError(m_CallContext.Paren,
	    fmt::format("Native function '{}' expected argument {}.", m_FunctionName, index + 1));
}

NoctObject NativeContextFunction::Call(CallContext& ctx, const std::vector<NoctObject>& args) {
	NativeCallContext nativeContext { ctx, args, m_Name };
	return m_Handler(nativeContext);
}

Module::Module(std::string name)
    : m_Name(std::move(name)) {
}

Module& Module::context_function(std::string name, NativeContextFunction::Handler handler) {
	std::string key = name;
	m_Functions[std::move(key)] = std::make_shared<NativeContextFunction>(std::move(name), std::move(handler));
	return *this;
}

CallableRef Module::GetFunction(const Token& name) const {
	if (auto iter = m_Functions.find(name.Lexeme); iter != m_Functions.end()) {
		return iter->second;
	}

	throw RuntimeError(name, fmt::format("Module '{}' has no function '{}'", m_Name, name.Lexeme));
}

Module& ModuleRegistry::RegisterModule(std::string name) {
	auto [iter, inserted] = m_Modules.try_emplace(name, nullptr);
	if (inserted || !iter->second) {
		iter->second = std::make_shared<Module>(name);
	}

	return *iter->second;
}

ModuleRef ModuleRegistry::Find(std::string_view name) const {
	if (auto iter = m_Modules.find(std::string { name }); iter != m_Modules.end()) {
		return iter->second;
	}

	return nullptr;
}

}
