#pragma once

#include <filesystem>
#include <memory>
#include <vector>

#include "Context.h"
#include "Environment.h"

#include "lexer/NoctObject.h"
#include "lexer/Token.h"

#include "parser/expression/Expression.h"
#include "parser/expression/ExpressionFwd.h"
#include "parser/expression/FunctionValueFactory.h"
#include "parser/expression/ICallable.h"

#include "parser/expression/variants/Set.h"

#include "module/Module.h"

#include "parser/statement/ClassDecleration.h"
#include "parser/statement/FunctionDecleration.h"
#include "parser/statement/ReturnStatement.h"
#include "parser/statement/Statement.h"
#include "parser/statement/StatementFwd.h"

namespace Noct {

class Interpreter final {
public:
	Interpreter(Context& context)
	    : m_Context(context) { };

	Interpreter(Context& context, const ModuleRegistry& modules)
	    : m_Context(context)
	    , m_Modules(&modules) { };

	Interpreter(Context& context,
	    const ModuleRegistry& modules,
	    ScriptId currentScript,
	    std::filesystem::path currentFile,
	    std::vector<NoctComponentDeclaration>* components,
	    std::vector<NoctDiagnostic>* diagnostics)
	    : m_Context(context)
	    , m_Modules(&modules)
	    , m_CurrentScript(std::move(currentScript))
	    , m_CurrentFile(std::move(currentFile))
	    , m_Components(components)
	    , m_Diagnostics(diagnostics) { };

	void operator()(const Unary&);
	void operator()(const Binary&);
	void operator()(const Literal&);
	void operator()(const This&);
	void operator()(const Ternary&);
	void operator()(const Grouping&);
	void operator()(const Variable&);
	void operator()(const Assign&);
	void operator()(const Logical&);
	void operator()(const Call&);
	void operator()(const Maybe&);
	void operator()(const Get&);
	void operator()(const Set&);
	void operator()(const Lambda&);

	void operator()(const ExpressionStatement&);
	void operator()(const PrintStatement&);
	void operator()(const VariableDecleration&);
	void operator()(const ClassDecleration&);
	void operator()(const ImportStatement&);
	void operator()(FunctionDecleration&);
	void operator()(const BlockStatement&);
	void operator()(const IfStatement&);
	void operator()(const WhileStatement&);
	void operator()(const ReturnStatement&);
	void operator()(const BreakStatement&);

	void SetGlobalEnvironment(const std::shared_ptr<Environment>& env);
	void Interpret(const StatementPtrVector& statements);

	NoctObject InvokeFunction(FunctionValue& fn, const std::vector<NoctObject>& args, const Token& callSite);

	bool IsEqual(const NoctObject& left, const NoctObject& right);

	void EnsureNumbers(const Token& op, double* operand);
	void EnsureNumbers(const Token& op, double* operand, double* operand2);

	NoctObject GetLiteral() const { return m_Value; }

	void Evaluate(Expression& exp);
	void Execute(Statement& exp);

private:
	NoctObject ApplyAssignmentOperator(const Token& op, const NoctObject& current, const NoctObject& value);
	static const ModuleRegistry& EmptyModuleRegistry();

	struct EnvGuard {
		Interpreter& I;
		std::shared_ptr<Environment> saved;
		explicit EnvGuard(Interpreter& I, std::shared_ptr<Environment> next)
		    : I(I)
		    , saved(I.m_Env) { I.m_Env = std::move(next); }
		~EnvGuard() { I.m_Env = std::move(saved); }

		EnvGuard(const EnvGuard&) = delete;
		EnvGuard& operator=(const EnvGuard&) = delete;
	};

private:
	Context& m_Context;
	const ModuleRegistry* m_Modules { &EmptyModuleRegistry() };
	ScriptId m_CurrentScript {};
	std::filesystem::path m_CurrentFile {};
	std::vector<NoctComponentDeclaration>* m_Components {};
	std::vector<NoctDiagnostic>* m_Diagnostics {};
	NoctObject m_Value {};
	std::shared_ptr<Environment> m_Env { nullptr };
};

}
