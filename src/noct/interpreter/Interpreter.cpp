#include <iostream>
#include <memory>
#include <random>
#include <stdexcept>
#include <string>
#include <variant>

#include "noct/interpreter/Interpreter.h"

#include "noct/exceptions/BreakException.h"
#include "noct/exceptions/ReturnException.h"
#include "noct/exceptions/RuntimeException.h"

#include "noct/lexer/NoctObject.h"
#include "noct/lexer/Token.h"
#include "noct/lexer/TokenType.h"

#include "noct/parser/expression/Expression.h"
#include "noct/parser/expression/FunctionValue.h"
#include "noct/parser/expression/LiteralBoolifier.h"
#include "noct/parser/expression/LiteralStringifier.h"

#include "noct/parser/statement/BlockStatement.h"
#include "noct/parser/statement/BreakStatement.h"
#include "noct/parser/statement/ExpressionStatement.h"
#include "noct/parser/statement/FunctionDecleration.h"
#include "noct/parser/statement/IfStatement.h"
#include "noct/parser/statement/PrintStatement.h"
#include "noct/parser/statement/ReturnStatement.h"
#include "noct/parser/statement/Statement.h"
#include "noct/parser/statement/VariableDecleration.h"
#include "noct/parser/statement/WhileStatement.h"

namespace Noct {

void Interpreter::SetGlobalEnvironment(const std::shared_ptr<Environment>& env) {
	m_Env = env;
}

void Interpreter::Interpret(const StatementPtrVector& statements) {
	for (const auto& stmt : statements) {
		Execute(*stmt);
	}
}

void Interpreter::operator()(const ExpressionStatement& expr) {
	Evaluate(*expr.Expr);
}

void Interpreter::operator()(const PrintStatement& expr) {
	Evaluate(*expr.PrintExpr);
	std::cout << std::visit(LiteralStringifier {}, m_Value) << std::endl;
}

void Interpreter::operator()(const VariableDecleration& decl) {
	NoctObject obj {};

	if (decl.Initialiser) {
		Evaluate(*decl.Initialiser);
		obj = m_Value;
	}

	bool isInitialised { decl.Initialiser != nullptr };
	m_Env->Define(decl.Slot, obj, isInitialised);
}

void Interpreter::operator()(const BlockStatement& blockStmt) {
	Environment* previous = m_Env.get();
	auto local = std::make_unique<Environment>(blockStmt.LocalCount, previous);

	auto saved = std::move(m_Env);
	m_Env = std::move(local);

	for (const auto& stmt : blockStmt.Statements)
		Execute(*stmt);

	m_Env = std::move(saved);
}

void Interpreter::operator()(const BreakStatement&) {
	throw BreakException {};
}

void Interpreter::operator()(const ReturnStatement& stmt) {
	if (stmt.ReturnExpr)
		Evaluate(*stmt.ReturnExpr);
	else
		m_Value = std::monostate {};

	throw ReturnException { m_Value };
}

void Interpreter::operator()(const WhileStatement& whileStmt) {
	Evaluate(*whileStmt.Condition);

	while (std::visit(LiteralBoolifier {}, m_Value)) {
		try {
			Execute(*whileStmt.LoopGuts);
		} catch (const BreakException&) {
			break;
		}
		Evaluate(*whileStmt.Condition);
	}
}

void Interpreter::operator()(const IfStatement& ifStmt) {
	Evaluate(*ifStmt.Condition);
	bool condition { std::visit(LiteralBoolifier {}, m_Value) };

	if (condition) {
		Execute(*ifStmt.TrueStatement);
	} else if (ifStmt.FalseStatement) {
		Execute(*ifStmt.FalseStatement);
	}
}

void Interpreter::operator()(FunctionDecleration& fn) {
	NoctObject value {
		std::make_shared<FunctionValue>(
		    &fn.Body,
		    fn.Parameters,
		    fn.Name,
		    fn.LocalCount,
		    m_Env)
	};

	m_Env->Define(fn.Slot, value, true);
}

void Interpreter::operator()(const Literal& literal) {
	m_Value = literal.Value;
}

void Interpreter::operator()(const Grouping& group) {
	Evaluate(*group.GroupExpr);
}

void Interpreter::operator()(const Unary& unary) {
	Evaluate(*unary.Right);
	auto right { m_Value };
	auto rightDoublePtr { std::get_if<double>(&m_Value) };

	switch (unary.Operator.Type) {
	case TokenType::PlusPlus: {
		try {
			EnsureNumbers(unary.Operator, rightDoublePtr);
			Expression& expr { *unary.Right };
			auto var { std::get<Variable>(expr.Value) };
			m_Env->Assign(var.Slot, var.Depth, *rightDoublePtr + 1);
			m_Value = *rightDoublePtr + 1;
		} catch (std::bad_variant_access& e) {
			throw std::runtime_error("Can not increment non variable expression");
		}
		break;
	}
	case TokenType::MinusMinus: {
		try {

			EnsureNumbers(unary.Operator, rightDoublePtr);
			Expression& expr { *unary.Right };
			auto var { std::get<Variable>(expr.Value) };
			m_Env->Assign(var.Slot, var.Depth, *rightDoublePtr - 1);
			m_Value = *rightDoublePtr - 1;
		} catch (std::bad_variant_access& e) {
			throw std::runtime_error("Can not increment non variable expression");
		}
		break;
	}
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

void Interpreter::operator()(const Maybe&) {
	static std::mt19937 rng { std::random_device {}() };
	static std::bernoulli_distribution dis(0.5);
	m_Value = static_cast<bool>(dis(rng));
}

void Interpreter::operator()(const Lambda& lam) {
	m_Value = std::make_shared<FunctionValue>(
	    &lam.Body,
	    lam.Parameters,
	    std::nullopt,
	    lam.LocalCount,
	    m_Env);
}

void Interpreter::operator()(const Binary& binary) {
	Evaluate(*binary.Left);
	auto left = m_Value;

	Evaluate(*binary.Right);
	auto right = m_Value;

	auto rightDoublePtr = std::get_if<double>(&right);
	auto leftDoublePtr = std::get_if<double>(&left);

	auto rightStrPtr = std::get_if<std::string>(&right);
	auto leftStrPtr = std::get_if<std::string>(&left);

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

	case TokenType::Percentage:
		EnsureNumbers(binary.Operator, rightDoublePtr, leftDoublePtr);
		m_Value = static_cast<double>(
		    static_cast<int>(*leftDoublePtr) % static_cast<int>(*rightDoublePtr));
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

	case TokenType::Comma:
		m_Value = right;
		break;

	default:
		m_Value = std::monostate {};
		break;
	}
}

void Interpreter::operator()(const Ternary& ternary) {
	Evaluate(*ternary.Condition);
	bool ternaryResult { std::visit(LiteralBoolifier {}, m_Value) };

	if (ternaryResult) {
		Evaluate(*ternary.Left);
	} else {
		Evaluate(*ternary.Right);
	}
}

void Interpreter::operator()(const Variable& var) {
	m_Value = m_Env->Get(var.Slot, var.Depth);
}

void Interpreter::operator()(const Assign& exp) {
	Evaluate(*exp.Value);
	m_Env->Assign(exp.Slot, exp.Depth, m_Value);
}

void Interpreter::operator()(const Logical& exp) {
	Evaluate(*exp.Left);
	bool left = std::visit(LiteralBoolifier {}, m_Value);

	if (exp.Operator.Type == TokenType::And) {
		if (!left)
			return;
		Evaluate(*exp.Right);
		return;
	}

	if (left)
		return;
	Evaluate(*exp.Right);
}

void Interpreter::operator()(const Call& exp) {
	Evaluate(*exp.Callee);

	auto fnPtr = std::get_if<FunctionRef>(&m_Value);
	if (!fnPtr || !(*fnPtr)) {
		throw "Can only call functions.";
	}

	auto fn = *fnPtr;

	if (exp.Arguments.size() > fn->ParameterNames.size()) {
		auto errorMsg { std::format("Function expects {} arguments or less", fn->ParameterNames.size()) };
		throw 1;
	}

	auto local = std::make_shared<Environment>(fn->LocalCount, fn->Closure.get());

	for (size_t i = 0; i < fn->ParameterNames.size(); i++) {
		Evaluate(*exp.Arguments[i]);
		local->Define(i, m_Value, true);
	}

	auto saved = std::move(m_Env);
	m_Env = std::move(local);

	if (exp.Arguments.size() == fn->ParameterNames.size()) {
		for (const auto& stmt : *fn->Body) {
			try {
				Execute(*stmt);
			} catch (const ReturnException& e) {
				m_Value = e.GetObject();
				break;
			}
		}
	} else {
		std::vector missingSub(fn->ParameterNames.begin() + exp.Arguments.size(), fn->ParameterNames.end());
		m_Value = std::make_shared<FunctionValue>(
		    fn->Body,
		    missingSub,
		    std::nullopt,
		    fn->LocalCount,
		    m_Env);
	}

	m_Env = std::move(saved);
}

void Interpreter::Evaluate(Expression& exp) {
	std::visit(*this, exp.Value);
}

void Interpreter::Execute(Statement& stmt) {
	std::visit(*this, stmt.Instruction);
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

bool Interpreter::IsEqual(const NoctObject& left, const NoctObject& right) {
	if (left.index() != right.index())
		return false;

	return std::visit([&](const auto& value) {
		using T = std::decay_t<decltype(value)>;
		return value == std::get<T>(right);
	},
	    left);
}

}
