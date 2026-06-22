#include <fmt/format.h>

#include <iostream>
#include <memory>
#include <random>
#include <string>
#include <variant>

#include "interpreter/Interpreter.h"

#include "exceptions/BreakException.h"
#include "exceptions/ReturnException.h"
#include "exceptions/RuntimeException.h"

#include "lexer/NoctObject.h"
#include "lexer/Token.h"
#include "lexer/TokenType.h"

#include "parser/expression/ClassCallable.h"
#include "parser/expression/Expression.h"
#include "parser/expression/ExpressionFwd.h"
#include "parser/expression/FunctionValue.h"
#include "parser/expression/ICallable.h"
#include "parser/expression/LiteralBoolifier.h"
#include "parser/expression/LiteralStringifier.h"
#include "parser/expression/UserFunctionCallable.h"

#include "module/Module.h"

#include "parser/statement/BlockStatement.h"
#include "parser/statement/BreakStatement.h"
#include "parser/statement/ClassDecleration.h"
#include "parser/statement/ExpressionStatement.h"
#include "parser/statement/FunctionDecleration.h"
#include "parser/statement/IfStatement.h"
#include "parser/statement/ImportStatement.h"
#include "parser/statement/PrintStatement.h"
#include "parser/statement/ReturnStatement.h"
#include "parser/statement/Statement.h"
#include "parser/statement/VariableDecleration.h"
#include "parser/statement/WhileStatement.h"

namespace Noct {

void Interpreter::SetGlobalEnvironment(const std::shared_ptr<Environment>& env) {
	m_Env = env;
}

void Interpreter::Interpret(const StatementPtrVector& statements) {
	try {
		for (const auto& stmt : statements)
			Execute(*stmt);
	} catch (const RuntimeError& e) {
		m_Context.ReportRuntimeError(e);
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

	bool is_initialised { decl.Initialiser != nullptr };
	m_Env->Define(decl.Slot, obj, is_initialised);
}

void Interpreter::operator()(const ClassDecleration& classDecl) {
	ClassValueRef class_value_ref { std::make_shared<ClassValue>(classDecl.Name.Lexeme) };

	for (const auto& method : classDecl.Methods) {
		auto fn_value_factory { FunctionValueFactory { m_Env } };
		FunctionValueRef value = fn_value_factory(method);
		class_value_ref->Methods.emplace(method.Name.Lexeme, std::move(value));
	}

	CallableRef class_callable = std::make_shared<ClassCallable>(class_value_ref);

	m_Env->Define(classDecl.Slot, NoctObject { class_callable }, true);
}

void Interpreter::operator()(const ImportStatement& import) {
	ModuleRef module = m_Modules->Find(import.Name.Lexeme);
	if (!module) {
		throw RuntimeError(import.Name, fmt::format("Unknown module '{}'", import.Name.Lexeme));
	}

	m_Env->Define(import.Slot, NoctObject { module }, true);
}

void Interpreter::operator()(const BlockStatement& blockStmt) {
	auto local = std::make_shared<Environment>(blockStmt.FrameSize, m_Env);
	EnvGuard g(*this, local);
	for (const auto& stmt : blockStmt.Statements)
		Execute(*stmt);
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
	FunctionValueRef f = FunctionValueFactory { m_Env }(fn);
	CallableRef callable = std::make_shared<UserFunctionCallable>(f);
	m_Env->Define(fn.Slot, NoctObject { callable }, true);
}

void Interpreter::operator()(const Literal& literal) {
	m_Value = literal.Value;
}

void Interpreter::operator()(const Grouping& group) {
	Evaluate(*group.GroupExpr);
}

void Interpreter::operator()(const Get& get) {
	Evaluate(*get.Instance);
	if (const auto* module = std::get_if<ModuleRef>(&m_Value)) {
		if (!*module) {
			throw RuntimeError(get.Name, "Internal error: null module reference.");
		}

		m_Value = NoctObject { (*module)->GetFunction(get.Name) };
		return;
	}

	ClassInstanceRef* instance_ptr { std::get_if<ClassInstanceRef>(&m_Value) };

	if (!instance_ptr) {
		throw RuntimeError(get.Name, "Only instances can have properties.");
	}

	m_Value = (*instance_ptr)->Get(get.Name);
}

void Interpreter::operator()(const Set& set) {
	Evaluate(*set.Instance);

	auto instance = std::get_if<ClassInstanceRef>(&m_Value);
	if (!instance) {
		throw RuntimeError(set.Name, "Only instances can have properties.");
	}

	ClassInstanceRef inst = *instance;
	NoctObject current {};
	if (set.Operator.Type != TokenType::Equal) {
		current = inst->Get(set.Name);
	}

	Evaluate(*set.Value);

	m_Value = ApplyAssignmentOperator(set.Operator, current, m_Value);
	inst->Set(set.Name, m_Value);
}

void Interpreter::operator()(const Unary& unary) {
	Evaluate(*unary.Right);
	auto right { m_Value };
	auto right_double_ptr { std::get_if<double>(&m_Value) };

	switch (unary.Operator.Type) {
	case TokenType::PlusPlus: {
		try {
			EnsureNumbers(unary.Operator, right_double_ptr);
			Expression& expr { *unary.Right };
			auto var { std::get<Variable>(expr.Value) };
			m_Env->Assign(var.Slot, var.Depth, *right_double_ptr + 1);
			m_Value = *right_double_ptr + 1;
		} catch (std::bad_variant_access& e) {
			throw RuntimeError(unary.Operator, "Can not increment non variable expression");
		}
		break;
	}
	case TokenType::MinusMinus: {
		try {
			EnsureNumbers(unary.Operator, right_double_ptr);
			Expression& expr { *unary.Right };
			auto var { std::get<Variable>(expr.Value) };
			m_Env->Assign(var.Slot, var.Depth, *right_double_ptr - 1);
			m_Value = *right_double_ptr - 1;
		} catch (std::bad_variant_access& e) {
			throw RuntimeError(unary.Operator, "Can not decrement non variable expression");
		}
		break;
	}
	case TokenType::Minus:
		EnsureNumbers(unary.Operator, right_double_ptr);
		m_Value = -(*right_double_ptr);
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

void Interpreter::operator()(const Lambda& lambda) {
	FunctionValueRef f = FunctionValueFactory { m_Env }(lambda);
	CallableRef callable = std::make_shared<UserFunctionCallable>(f);
	m_Value = NoctObject { callable };
}

void Interpreter::operator()(const Binary& binary) {
	Evaluate(*binary.Left);
	auto left = m_Value;

	Evaluate(*binary.Right);
	auto right = m_Value;

	auto right_double_ptr = std::get_if<double>(&right);
	auto left_double_ptr = std::get_if<double>(&left);

	auto right_str_ptr = std::get_if<std::string>(&right);
	auto left_str_ptr = std::get_if<std::string>(&left);

	m_Value = std::monostate {};

	switch (binary.Operator.Type) {
	case TokenType::Minus:
		EnsureNumbers(binary.Operator, right_double_ptr, left_double_ptr);
		m_Value = *left_double_ptr - *right_double_ptr;
		break;

	case TokenType::Plus: {
		if (right_str_ptr && left_str_ptr) {
			m_Value = *left_str_ptr + *right_str_ptr;
			break;
		}
		if (right_str_ptr) {
			m_Value = std::visit(LiteralStringifier {}, left) + *right_str_ptr;
			break;
		}
		if (left_str_ptr) {
			m_Value = *left_str_ptr + std::visit(LiteralStringifier {}, right);
			break;
		}
		if (left_double_ptr && right_double_ptr) {
			m_Value = *left_double_ptr + *right_double_ptr;
			break;
		}
		throw RuntimeError(binary.Operator, "Operands must be two numbers or at least one strings");
	}

	case TokenType::Slash:
		EnsureNumbers(binary.Operator, right_double_ptr, left_double_ptr);
		if (*right_double_ptr == 0) {
			throw RuntimeError(binary.Operator, "Can not divide by 0");
		}
		m_Value = *left_double_ptr / *right_double_ptr;
		break;

	case TokenType::Percentage:
		EnsureNumbers(binary.Operator, right_double_ptr, left_double_ptr);
		if (static_cast<int>(*right_double_ptr) == 0) {
			throw RuntimeError(binary.Operator, "Can not divide by 0");
		}
		m_Value = static_cast<double>(
		    static_cast<int>(*left_double_ptr) % static_cast<int>(*right_double_ptr));
		break;

	case TokenType::Star:
		EnsureNumbers(binary.Operator, right_double_ptr, left_double_ptr);
		m_Value = *left_double_ptr * *right_double_ptr;
		break;

	case TokenType::Greater:
		EnsureNumbers(binary.Operator, right_double_ptr, left_double_ptr);
		m_Value = *left_double_ptr > *right_double_ptr;
		break;

	case TokenType::Less:
		EnsureNumbers(binary.Operator, right_double_ptr, left_double_ptr);
		m_Value = *left_double_ptr < *right_double_ptr;
		break;

	case TokenType::LessEqual:
		EnsureNumbers(binary.Operator, right_double_ptr, left_double_ptr);
		m_Value = *left_double_ptr <= *right_double_ptr;
		break;

	case TokenType::GreaterEqual:
		EnsureNumbers(binary.Operator, right_double_ptr, left_double_ptr);
		m_Value = *left_double_ptr >= *right_double_ptr;
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
	bool ternary_result { std::visit(LiteralBoolifier {}, m_Value) };

	if (ternary_result) {
		Evaluate(*ternary.Left);
	} else {
		Evaluate(*ternary.Right);
	}
}

void Interpreter::operator()(const Variable& var) {
	if (var.Slot == UNRESOLVED || var.Depth == UNRESOLVED)
		throw RuntimeError(var.Name, "Internal error: unresolved variable access.");
	m_Value = m_Env->Get(var.Slot, var.Depth);
}

void Interpreter::operator()(const This& expr) {
	if (expr.Slot == UNRESOLVED || expr.Depth == UNRESOLVED)
		throw RuntimeError(expr.Keyword, "Internal error: unresolved 'this'.");

	m_Value = m_Env->Get(expr.Slot, expr.Depth);
}

void Interpreter::operator()(const Assign& expr) {
	if (expr.Slot == UNRESOLVED || expr.Depth == UNRESOLVED)
		throw RuntimeError(expr.Name, "Internal error: unresolved assignment.");

	NoctObject current {};
	if (expr.Operator.Type != TokenType::Equal) {
		current = m_Env->Get(expr.Slot, expr.Depth);
	}

	Evaluate(*expr.Value);
	m_Value = ApplyAssignmentOperator(expr.Operator, current, m_Value);
	m_Env->Assign(expr.Slot, expr.Depth, m_Value);
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

	auto callable_ptr = std::get_if<CallableRef>(&m_Value);
	if (!callable_ptr || !*callable_ptr) {
		throw RuntimeError(exp.Paren, "Can only call functions and classes.");
	}

	CallableRef callee = *callable_ptr;

	std::vector<NoctObject> args;
	args.reserve(exp.Arguments.size());
	for (auto& a : exp.Arguments) {
		Evaluate(*a);
		args.push_back(m_Value);
	}

	if (!callee->IsVariadic() && args.size() != callee->Arity()) {
		auto error_str { fmt::format("{} expects {} arguments but received {}", callee->Name(), callee->Arity(), args.size()) };
		throw RuntimeError(exp.Paren, error_str);
	}

	CallContext ctx {
		.I = *this,
		.Paren = exp.Paren,
		.CurrentScript = m_CurrentScript,
		.CurrentFile = m_CurrentFile,
		.Diagnostics = m_Diagnostics,
		.Components = m_Components,
	};
	m_Value = callee->Call(ctx, args);
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

NoctObject Interpreter::ApplyAssignmentOperator(const Token& op, const NoctObject& current, const NoctObject& value) {
	switch (op.Type) {
	case TokenType::Equal:
		return value;

	case TokenType::PlusEqual: {
		const auto* current_number = std::get_if<double>(&current);
		const auto* value_number = std::get_if<double>(&value);
		const auto* current_string = std::get_if<std::string>(&current);
		const auto* value_string = std::get_if<std::string>(&value);

		if (current_string && value_string)
			return *current_string + *value_string;
		if (current_string)
			return *current_string + std::visit(LiteralStringifier {}, value);
		if (value_string)
			return std::visit(LiteralStringifier {}, current) + *value_string;
		if (current_number && value_number)
			return *current_number + *value_number;

		throw RuntimeError(op, "Operands must be two numbers or at least one strings");
	}

	case TokenType::MinusEqual: {
		const auto* current_number = std::get_if<double>(&current);
		const auto* value_number = std::get_if<double>(&value);
		if (current_number && value_number)
			return *current_number - *value_number;

		throw RuntimeError(op, "Operands must be a number.");
	}

	default:
		throw RuntimeError(op, "Internal error: unsupported assignment operator.");
	}
}

const ModuleRegistry& Interpreter::EmptyModuleRegistry() {
	static const ModuleRegistry registry {};
	return registry;
}

NoctObject Interpreter::InvokeFunction(FunctionValue& fn,
    const std::vector<NoctObject>& args,
    const Token& callSite) {

	const auto expected_amount = fn.ParameterNames.size();
	if (args.size() != expected_amount) {
		throw RuntimeError(callSite,
		    fmt::format("Function expects {} arguments but received {}",
		        expected_amount, args.size()));
	}

	auto local = std::make_shared<Environment>(fn.FrameSize, fn.Closure);

	for (size_t i = 0; i < expected_amount; ++i)
		local->Define(i, args[i], true);

	EnvGuard guard(*this, local);

	NoctObject result { std::monostate {} };

	try {
		for (const auto& stmt : *fn.Body)
			Execute(*stmt);
	} catch (ReturnException& ret) {
		result = ret.GetObject();
	}

	return result;
}

}
