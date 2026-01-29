#include <cstddef>
#include <filesystem>
#include <fstream>

#include "Run.h"

#include "noct/Context.h"
#include "noct/Logger.h"
#include "noct/RunResult.h"

#include "noct/interpreter/Interpreter.h"

#include "noct/lexer/Lexer.h"

#include "noct/parser/Parser.h"

#include "noct/resolver/Resolver.h"

namespace Noct {

namespace {
	int RunFile(const std::filesystem::path& path, Context& context) {
		std::ifstream f { path };

		if (!f.is_open()) {
			Logger::Error("Unable to open {} ", path.c_str());
			return 1;
		}

		std::string line {};
		std::string contents {};

		while (std::getline(f, line))
			contents.append(line).append("\n");

		RunResult result { RunFromString(context, contents) };

		return result.HadParseError || result.HadSemanticError || result.HadRuntimeError;
	}

}

RunResult RunFromString(Context& context, std::string_view contents) {
	Logger::detail::SetEnabled(context.LoggingEnabled);
	Lexer lexer { contents, context };
	Parser parser { lexer.ScanTokens(), context };
	RunResult result {};

	auto statements = parser.Parse();

	if (context.HadParseError) {
		result.HadParseError = true;
		return result;
	}

	if (!statements.size()) {
		return result;
	}

	Resolver resolver { context };
	resolver.Resolve(statements);

	if (context.HadSemanticError) {
		result.HadSemanticError = true;
		return result;
	}

	Interpreter interpreter { context };
	interpreter.SetGlobalEnvironment(std::make_shared<Environment>(resolver.GetGlobalFrameSize()));
	interpreter.Interpret(statements);

	if (context.HadRuntimeError) {
		result.HadRuntimeError = true;
		return result;
	}
	result.Value = interpreter.GetLiteral();

	return result;
}

int Run(int argc, char** argv) {
	Context ctx {
		.Args = { argv, static_cast<std::size_t>(argc) }
	};

	switch (argc) {
	case 2: {
		auto path { std::filesystem::path(ctx.Args[1]) };
		return RunFile(path, ctx);
		break;
	}
	default:
		Logger::Error("Usage: {} [script_path]", ctx.Args[0]);
		return 1;
	}
}

}
