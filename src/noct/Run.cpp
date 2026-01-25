#include <cstddef>
#include <filesystem>
#include <fstream>

#include "Run.h"

#include "noct/Context.h"
#include "noct/Logger.h"

#include "noct/lexer/Lexer.h"

#include "noct/parser/Interpreter.h"
#include "noct/parser/Parser.h"

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

		auto statements = parser.Parse();

		if (!statements.size()) {
			Logger::Info("No statements");
			return;
		}

		Interpreter interpreter { context };
		interpreter.Interpret(statements);
	}

	static void RunPrompt(Context& context) {
		Logger::Info("Entering interactive mode");
		while (!context.HadError) {
			std::string inp;
			std::cout << ">> ";
			std::cin >> inp;

			Lexer lexer { inp, context };
			Parser parser { lexer.ScanTokens(), context };

			auto statements = parser.Parse();

			if (!statements.size()) {
				Logger::Info("No statements");
				return;
			}

			Interpreter interpreter { context };
			interpreter.Interpret(statements);
		}
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
