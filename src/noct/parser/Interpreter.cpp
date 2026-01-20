#include "noct/parser/Interpreter.h"

#include "noct/Logger.h"
#include "noct/exceptions/RuntimeException.h"
#include "noct/lexer/Token.h"
#include "noct/lexer/TokenType.h"
#include "noct/parser/Expression.h"
#include "noct/parser/Grouping.h"
#include "noct/parser/Literal.h"
#include "noct/parser/Unary.h"
#include "noct/parser/Binary.h"
#include "noct/parser/Ternary.h"

namespace Noct {

void Interpreter::Interpret(const Expression& exp) {
	try {
		Evaluate(exp);
		Logger::Info("");
		Logger::Info("{}", Stringify(m_Value));
	} catch (RuntimeError& error) {
		throw error;
	}
}

void Interpreter::Visit(const Grouping& group) {
	Evaluate(*group.GroupExpr);
}

void Interpreter::Visit(const Literal& literal) {
	m_Value = literal.Value;
}

void Interpreter::Visit(const Unary& unary) {
	Evaluate(*unary.Right);
	auto right { m_Value };
	auto rightDoublePtr { std::get_if<double>(&m_Value) };

	switch (unary.Operator.Type) {
	case TokenType::Minus:
		EnsureNumbers(unary.Operator, rightDoublePtr);
		m_Value = -(*rightDoublePtr);
		break;
	case TokenType::Bang:
		m_Value = !IsTruthy(right);
		break;
	default:
		m_Value = std::monostate {};
		break;
	}
}

void Interpreter::Visit(const Binary& binary) {
	Evaluate(*binary.Left);
	auto left = m_Value;

	Evaluate(*binary.Right);
	auto right = m_Value;

	auto rightDoublePtr = std::get_if<double>(&right);
	auto leftDoublePtr = std::get_if<double>(&left);

	m_Value = std::monostate {};

	switch (binary.Operator.Type) {
	case TokenType::Minus:
		EnsureNumbers(binary.Operator, rightDoublePtr, leftDoublePtr);
		m_Value = *leftDoublePtr - *rightDoublePtr;
		break;
	case TokenType::Plus: {
		auto rightStrPtr = std::get_if<std::string>(&right);
		auto leftStrPtr = std::get_if<std::string>(&left);

		if (rightStrPtr && leftStrPtr) {
			m_Value = *leftStrPtr + *rightStrPtr;
			return;
		}

		if (leftDoublePtr && rightDoublePtr) {
			m_Value = *leftDoublePtr + *rightDoublePtr;
			return;
		}

		throw RuntimeError(binary.Operator, "Operands must be two numbers or two strings");
	}
	case TokenType::Slash:
		EnsureNumbers(binary.Operator, rightDoublePtr, leftDoublePtr);
		m_Value = *leftDoublePtr / *rightDoublePtr;
		break;
	case TokenType::Star:
		EnsureNumbers(binary.Operator, rightDoublePtr, leftDoublePtr);
		m_Value = *leftDoublePtr * *rightDoublePtr;
		break;
	case TokenType::Greater:
		EnsureNumbers(binary.Operator, rightDoublePtr, leftDoublePtr);
		m_Value = *leftDoublePtr > *rightDoublePtr;
		break;
	case TokenType::Less:
		EnsureNumbers(binary.Operator, rightDoublePtr, leftDoublePtr);
		m_Value = *leftDoublePtr < *rightDoublePtr;
		break;
	case TokenType::LessEqual:
		EnsureNumbers(binary.Operator, rightDoublePtr, leftDoublePtr);
		m_Value = *leftDoublePtr <= *rightDoublePtr;
		break;
	case TokenType::GreaterEqual:
		EnsureNumbers(binary.Operator, rightDoublePtr, leftDoublePtr);
		m_Value = *leftDoublePtr >= *rightDoublePtr;
		break;
	case TokenType::BangEqual:
		m_Value = !IsEqual(left, right);
		break;
	case TokenType::EqualEqual:
		m_Value = IsEqual(left, right);
		break;
	case Noct::TokenType::Comma:
		m_Value = right;
		break;
	default:
		m_Value = std::monostate {};
	}
}

void Interpreter::Visit(const Ternary& ternary) {
	Evaluate(*ternary.Condition);
	bool ternaryResult { IsTruthy(m_Value) };

	if (ternaryResult) {
		Evaluate(*ternary.Left);
	} else {
		Evaluate(*ternary.Right);
	}
}

void Interpreter::Evaluate(const Expression& exp) {
	exp.Accept(*this);
}

bool Interpreter::IsTruthy(const NoctLiteral& literal) {
	if (std::get_if<std::monostate>(&literal)) {
		return false;
	}

	if (auto boolPtr { std::get_if<bool>(&literal) }) {
		return *boolPtr;
	}

	return true;
}

void Interpreter::EnsureNumbers(const Token& op, double* operand) {
	if (operand)
		return;

	throw RuntimeError(op, "Operand must be a number.");
}

void Interpreter::EnsureNumbers(const Token& op, double* operand1, double* operand2) {
	if (operand1 && operand2)
		return;

	throw RuntimeError(op, "Operands must be a number.");
}

bool Interpreter::IsEqual(const NoctLiteral& left, const NoctLiteral& right) {
	if (left.index() != right.index())
		return false;

	return std::visit([&](const auto& value) {
		using T = std::decay_t<decltype(value)>;
		return value == std::get<T>(right);
	},
	    left);
}

std::string Interpreter::Stringify(const NoctLiteral& literal) const {
	if (literal.index() == 0)
		return "nil";

	return std::visit([](const auto& value) -> std::string {
		using T = std::decay_t<decltype(value)>;

		if constexpr (std::is_same_v<T, std::string>) {
			return value;
		} else if constexpr (std::is_same_v<T, bool>) {
			return value ? "true" : "false";
		} else if constexpr (std::is_arithmetic_v<T>) {
			return std::to_string(value);
		} else {
			return "";
		}
	},
	    literal);
}

}
