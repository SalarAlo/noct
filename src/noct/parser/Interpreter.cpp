#include "noct/parser/Interpreter.h"

#include <iostream>
#include <memory>
#include <string>
#include <variant>

#include "noct/exceptions/RuntimeException.h"
#include "noct/lexer/Token.h"
#include "noct/lexer/TokenType.h"

#include "noct/parser/expression/Assign.h"
#include "noct/parser/expression/LiteralBoolifier.h"
#include "noct/parser/statement/BlockStatement.h"
#include "noct/parser/statement/ExpressionStatement.h"

#include "noct/parser/expression/Expression.h"
#include "noct/parser/expression/Grouping.h"
#include "noct/parser/expression/Literal.h"
#include "noct/parser/expression/LiteralStringifier.h"
#include "noct/parser/expression/Unary.h"
#include "noct/parser/expression/Logical.h"
#include "noct/parser/expression/Binary.h"
#include "noct/parser/expression/Ternary.h"
#include "noct/parser/expression/Variable.h"

#include "noct/parser/statement/IfStatement.h"
#include "noct/parser/statement/Statement.h"
#include "noct/parser/statement/ExpressionStatement.h"
#include "noct/parser/statement/BlockStatement.h"
#include "noct/parser/statement/PrintStatement.h"
#include "noct/parser/statement/VariableDecleration.h"
#include "noct/parser/statement/WhileStatement.h"

namespace Noct {

void Interpreter::Interpret(const std::vector<std::unique_ptr<Statement>>& statements) {
	try {
		for (const auto& stmt : statements) {
			Execute(*stmt);
		}
	} catch (RuntimeError& error) {
		m_Context.RegisterTokenError(error.GetToken(), error.what());
	}
}

void Interpreter::Visit(const Grouping& group) {
	Evaluate(*group.GroupExpr);
}

void Interpreter::Visit(const ExpressionStatement& expr) {
	Evaluate(*expr.Expr);
}

void Interpreter::Visit(const PrintStatement& expr) {
	Evaluate(*expr.Expr);
	std::cout << std::visit(LiteralStringifier {}, m_Value) << std::endl;
}

void Interpreter::Visit(const VariableDecleration& decl) {
	NoctLiteral obj {};
	if (decl.Initialiser) {
		Evaluate(*decl.Initialiser);
		obj = m_Value;
	}

	bool isInitialised { decl.Initialiser != nullptr };
	m_Env->Define(decl.Name, obj, isInitialised);
}

void Interpreter::Visit(const BlockStatement& blockStmt) {
	Environment* previous { m_Env.release() };
	m_Env = std::make_unique<Environment>(previous);

	for (const auto& stmt : blockStmt.Statements) {
		Execute(*stmt);
	}

	m_Env.reset(previous);
}

void Interpreter::Visit(const WhileStatement& whileStmt) {
	Evaluate(*whileStmt.RunCondition);
	bool condition { std::visit(LiteralBoolifier {}, m_Value) };

	while (condition) {
		Execute(*whileStmt.ExecuteStatement);

		Evaluate(*whileStmt.RunCondition);
		condition = std::visit(LiteralBoolifier {}, m_Value);
	}
}

void Interpreter::Visit(const IfStatement& ifStmt) {
	Evaluate(*ifStmt.Condition);
	bool condition { std::visit(LiteralBoolifier {}, m_Value) };
	if (condition) {
		Execute(*ifStmt.TrueStatement);
	} else if (ifStmt.FalseStatement) {
		Execute(*ifStmt.FalseStatement);
	}
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
		m_Value = !std::visit(LiteralBoolifier {}, right);
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

	auto rightStrPtr = std::get_if<std::string>(&right);
	auto leftStrPtr = std::get_if<std::string>(&left);

	auto rightBoolPtr = std::get_if<bool>(&right);
	auto leftBoolPtr = std::get_if<bool>(&left);

	m_Value = std::monostate {};

	switch (binary.Operator.Type) {
	case TokenType::Minus:
		EnsureNumbers(binary.Operator, rightDoublePtr, leftDoublePtr);
		m_Value = *leftDoublePtr - *rightDoublePtr;
		break;
	case TokenType::Plus: {
		if (rightStrPtr && leftStrPtr) {
			m_Value = *leftStrPtr + *rightStrPtr;
			break;
		}

		if (rightStrPtr) {
			m_Value = std::visit(LiteralStringifier {}, left) + *rightStrPtr;
			break;
		}

		if (leftStrPtr) {
			m_Value = *leftStrPtr + std::visit(LiteralStringifier {}, right);
			break;
		}

		if (leftDoublePtr && rightDoublePtr) {
			m_Value = *leftDoublePtr + *rightDoublePtr;
			break;
		}

		throw RuntimeError(binary.Operator, "Operands must be two numbers or at least one strings");
	}
	case TokenType::Slash:
		EnsureNumbers(binary.Operator, rightDoublePtr, leftDoublePtr);

		if (*rightDoublePtr == 0) {
			throw RuntimeError(binary.Operator, "Right hand side of division operator is 0. Can not divide by 0");
		}

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
	bool ternaryResult { std::visit(LiteralBoolifier {}, m_Value) };

	if (ternaryResult) {
		Evaluate(*ternary.Left);
	} else {
		Evaluate(*ternary.Right);
	}
}

void Interpreter::Visit(const Variable& var) {
	m_Value = m_Env->Get(var.Name);
}

void Interpreter::Visit(const Assign& exp) {
	Evaluate(*exp.value);
	m_Env->Assign(exp.name, m_Value);
}

void Interpreter::Visit(const Logical& exp) {
	Evaluate(*exp.Left);
	bool left = std::visit(LiteralBoolifier {}, m_Value);

	if (exp.Operator.Type == TokenType::And) {
		if (!left)
			return;

		Evaluate(*exp.Right);
	} else if (exp.Operator.Type == TokenType::Or) {
		if (left)
			return;

		Evaluate(*exp.Right);
	}
}

void Interpreter::Evaluate(const Expression& exp) {
	exp.Accept(*this);
}

void Interpreter::Execute(const Statement& stmt) {
	stmt.Accept(*this);
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

}
