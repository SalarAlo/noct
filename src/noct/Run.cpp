#include "Run.h"

#include <cstddef>
#include <filesystem>
#include <fstream>
#include <variant>

#include "noct/Context.h"
#include "noct/Logger.h"
#include "noct/parser/Interpreter.h"
#include "noct/parser/Parser.h"
#include "noct/parser/PrintVisitor.h"
#include "noct/lexer/Lexer.h"

namespace Noct {

namespace {
	void RunFile(const std::filesystem::path& path, Context& context) {
		std::ifstream f { path };

		if (!f.is_open()) {
			Logger::Error("Unable to open {} ", path.c_str());
			context.HadError = true;
			return;
		}

		std::string line {};
		std::string contents {};

		while (std::getline(f, line))
			contents.append(line).append("\n");

		Lexer lexer { contents, context };
		Parser parser { lexer.ScanTokens(), context };
		PrintVisitor visitor {};

		auto ast = parser.Parse();

		if (!ast) {
			Logger::Info("No AST");
			return;
		}

		ast->Accept(visitor);

		Interpreter interpreter {};
		interpreter.Interpret(*ast);
	}

	static void RunPrompt(Context& ctx) {
		Logger::Info("Entering interactive mode");
	}
}

int Run(int argc, char** argv) {
	Context ctx {
		.Args = { argv, static_cast<std::size_t>(argc) }
	};

	Logger::Info("Context initialized");

	switch (argc) {
	case 1:
		RunPrompt(ctx);
		break;
	case 2: {
		auto path { std::filesystem::path(ctx.Args[1]) };
		RunFile(path, ctx);
		break;
	}
	default:
		Logger::Error("Usage: {} [script_path]", ctx.Args[0]);
		return 1;
	}

	return ctx.HadError ? 1 : 0;
}

}
