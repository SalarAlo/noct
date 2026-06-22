#pragma once

#include <fmt/format.h>

#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

#include "exceptions/RuntimeException.h"

#include "lexer/NoctObject.h"
#include "lexer/Token.h"

#include "parser/expression/ICallable.h"
#include "parser/expression/ModuleFwd.h"

namespace Noct {

class NativeCallContext {
public:
	NativeCallContext(CallContext& callContext, const std::vector<NoctObject>& args, std::string_view functionName)
	    : m_CallContext(callContext)
	    , m_Args(args)
	    , m_FunctionName(functionName) { }

	size_t argument_count() const { return m_Args.size(); }
	size_t ArgumentCount() const { return argument_count(); }
	const NoctObject& argument(size_t index) const;
	const NoctObject& Argument(size_t index) const { return argument(index); }

	std::string require_string(size_t index);
	double require_number(size_t index);
	bool require_bool(size_t index);
	NoctValue require_value(size_t index);

	void add_error(std::string message, std::string code = "native-binding");
	void add_component(NoctComponentDeclaration component);

	const ScriptId& script_id() const { return m_CallContext.CurrentScript; }
	NoctSourceLocation location() const;

private:
	const NoctObject& RequireArgument(size_t index) const;

private:
	CallContext& m_CallContext;
	const std::vector<NoctObject>& m_Args;
	std::string_view m_FunctionName;
};

namespace detail {
	template <typename>
	inline constexpr bool AlwaysFalse = false;

	template <typename T>
	struct FunctionTraits : FunctionTraits<decltype(&T::operator())> { };

	template <typename R, typename... Args>
	struct FunctionTraits<R (*)(Args...)> {
		using ReturnType = R;
		using ArgsTuple = std::tuple<Args...>;
	};

	template <typename R, typename... Args>
	struct FunctionTraits<std::function<R(Args...)>> {
		using ReturnType = R;
		using ArgsTuple = std::tuple<Args...>;
	};

	template <typename C, typename R, typename... Args>
	struct FunctionTraits<R (C::*)(Args...) const> {
		using ReturnType = R;
		using ArgsTuple = std::tuple<Args...>;
	};

	template <typename C, typename R, typename... Args>
	struct FunctionTraits<R (C::*)(Args...)> {
		using ReturnType = R;
		using ArgsTuple = std::tuple<Args...>;
	};

	template <typename T>
	T ConvertNativeArgument(const NoctObject& value, const Token& callSite, std::string_view functionName, size_t index) {
		using CleanT = std::remove_cvref_t<T>;

		if constexpr (std::is_same_v<CleanT, NoctObject>) {
			return value;
		} else if constexpr (std::is_same_v<CleanT, double>) {
			if (const auto* converted = std::get_if<double>(&value))
				return *converted;
		} else if constexpr (std::is_same_v<CleanT, std::string>) {
			if (const auto* converted = std::get_if<std::string>(&value))
				return *converted;
		} else if constexpr (std::is_same_v<CleanT, bool>) {
			if (const auto* converted = std::get_if<bool>(&value))
				return *converted;
		} else {
			static_assert(AlwaysFalse<T>, "Unsupported native function argument type");
		}

		throw RuntimeError(callSite,
		    fmt::format("Invalid argument {} for native function '{}'", index + 1, functionName));
	}

	template <typename T>
	NoctObject ConvertNativeReturn(T&& value) {
		using CleanT = std::remove_cvref_t<T>;

		if constexpr (std::is_same_v<CleanT, NoctObject>) {
			return std::forward<T>(value);
		} else if constexpr (std::is_same_v<CleanT, double>) {
			return value;
		} else if constexpr (std::is_integral_v<CleanT> && !std::is_same_v<CleanT, bool>) {
			return static_cast<double>(value);
		} else if constexpr (std::is_same_v<CleanT, std::string>) {
			return std::forward<T>(value);
		} else if constexpr (std::is_same_v<CleanT, const char*> || std::is_same_v<CleanT, char*>) {
			return std::string { value };
		} else if constexpr (std::is_same_v<CleanT, bool>) {
			return value;
		} else {
			static_assert(AlwaysFalse<T>, "Unsupported native function return type");
		}
	}
}

class NativeContextFunction final : public ICallable {
public:
	using Handler = std::function<NoctObject(NativeCallContext&)>;

	NativeContextFunction(std::string name, Handler handler)
	    : m_Name(std::move(name))
	    , m_Handler(std::move(handler)) { }

	size_t Arity() const override { return 0; }
	bool IsVariadic() const override { return true; }
	NoctObject Call(CallContext& ctx, const std::vector<NoctObject>& args) override;
	std::string_view Name() const override { return m_Name; }

private:
	std::string m_Name;
	Handler m_Handler;
};

template <typename Fn>
class NativeFunction final : public ICallable {
public:
	NativeFunction(std::string name, Fn fn)
	    : m_Name(std::move(name))
	    , m_Function(std::move(fn)) { }

	size_t Arity() const override {
		return std::tuple_size_v<typename detail::FunctionTraits<Fn>::ArgsTuple>;
	}

	NoctObject Call(CallContext& ctx, const std::vector<NoctObject>& args) override {
		return CallImpl(ctx, args, std::make_index_sequence<std::tuple_size_v<typename detail::FunctionTraits<Fn>::ArgsTuple>> {});
	}

	std::string_view Name() const override { return m_Name; }

private:
	template <size_t... I>
	NoctObject CallImpl(CallContext& ctx, const std::vector<NoctObject>& args, std::index_sequence<I...>) {
		using Traits = detail::FunctionTraits<Fn>;
		using ReturnType = typename Traits::ReturnType;
		using ArgsTuple = typename Traits::ArgsTuple;

		if constexpr (std::is_void_v<ReturnType>) {
			std::invoke(m_Function,
			    detail::ConvertNativeArgument<std::tuple_element_t<I, ArgsTuple>>(args[I], ctx.Paren, m_Name, I)...);
			return std::monostate {};
		} else {
			return detail::ConvertNativeReturn(std::invoke(m_Function,
			    detail::ConvertNativeArgument<std::tuple_element_t<I, ArgsTuple>>(args[I], ctx.Paren, m_Name, I)...));
		}
	}

private:
	std::string m_Name;
	Fn m_Function;
};

class Module {
public:
	explicit Module(std::string name);

	const std::string& Name() const { return m_Name; }

	template <typename Fn>
	Module& function(std::string name, Fn fn) {
		std::string key = name;
		m_Functions[std::move(key)] = std::make_shared<NativeFunction<std::decay_t<Fn>>>(std::move(name), std::forward<Fn>(fn));
		return *this;
	}

	Module& context_function(std::string name, NativeContextFunction::Handler handler);
	Module& ContextFunction(std::string name, NativeContextFunction::Handler handler) {
		return context_function(std::move(name), std::move(handler));
	}

	CallableRef GetFunction(const Token& name) const;

private:
	std::string m_Name;
	std::unordered_map<std::string, CallableRef> m_Functions;
};

class ModuleRegistry {
public:
	Module& RegisterModule(std::string name);
	Module& registerModule(std::string name) { return RegisterModule(std::move(name)); }

	ModuleRef Find(std::string_view name) const;

private:
	std::unordered_map<std::string, ModuleRef> m_Modules;
};

}
