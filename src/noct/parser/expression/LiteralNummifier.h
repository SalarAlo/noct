#pragma once

#include <optional>
#include <stdexcept>
#include <string>
#include <variant>

#include "noct/parser/expression/FunctionValue.h"

namespace Noct {
struct LiteralNumifier {
	std::optional<double> operator()(const double& value) const {
		return value;
	}

	std::optional<double> operator()(const std::string& value) const {
		try {
			double v = std::stod(value);
			return v;
		} catch (const std::invalid_argument&) {
			return {};
		} catch (const std::out_of_range&) {
			return {};
		}
	}

	std::optional<double> operator()(const bool& value) const {
		return value ? 1 : 0;
	}

	std::optional<double> operator()(const std::monostate&) const {
		return 0;
	}

	double operator()(const FunctionRef& f) const {
		return f->ParameterNames.size();
	}
};
}
