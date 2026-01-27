#pragma once

#include <fmt/core.h>

#include <iostream>
#include <string_view>
#include <utility>

namespace Noct::Logger {

namespace detail {
	inline static bool s_Enabled;

	inline void SetEnabled(bool enabled) {
		s_Enabled = enabled;
	}

	inline void write(std::string_view level, std::string_view message, bool error) {
		if (!s_Enabled)
			return;
		auto& out = error ? std::cerr : std::cout;
		out << "[" << level << "] " << message << '\n';
	}
}

template <typename... Args>
inline void Debug(fmt::format_string<Args...> fmt, Args&&... args) {
	detail::write("debug", fmt::format(fmt, std::forward<Args>(args)...), false);
}

template <typename... Args>
inline void Info(fmt::format_string<Args...> fmt, Args&&... args) {
	detail::write("info", fmt::format(fmt, std::forward<Args>(args)...), false);
}

template <typename... Args>
inline void Warn(fmt::format_string<Args...> fmt, Args&&... args) {
	detail::write("warn", fmt::format(fmt, std::forward<Args>(args)...), false);
}

template <typename... Args>
inline void Error(fmt::format_string<Args...> fmt, Args&&... args) {
	detail::write("error", fmt::format(fmt, std::forward<Args>(args)...), true);
}

}
