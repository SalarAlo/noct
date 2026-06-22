#include <cmath>
#include <string>

#include <doctest/doctest.h>

#include "Context.h"
#include "Runtime.h"

template <typename T>
static T RunAndGet(Noct::Runtime& runtime, std::string_view src) {
	Noct::Context ctx {};
	ctx.LoggingEnabled = false;

	Noct::RunResult r = runtime.RunString(ctx, src);

	REQUIRE(!r.HadParseError);
	REQUIRE(!r.HadSemanticError);
	REQUIRE(!r.HadRuntimeError);
	REQUIRE(std::holds_alternative<T>(r.Value));

	return std::get<T>(r.Value);
}

TEST_CASE("import math and call native function") {
	Noct::Runtime runtime {};
	runtime.registerModule("math")
	    .function("sqrt", [](double x) {
		    return std::sqrt(x);
	    });

	CHECK(RunAndGet<double>(runtime, R"(
		import math
		math.sqrt(25);
	)") == 5.0);
}

TEST_CASE("unknown imported module fails") {
	Noct::Runtime runtime {};
	Noct::Context ctx {};
	ctx.LoggingEnabled = false;

	auto r = runtime.RunString(ctx, "import missing");

	CHECK(r.HadRuntimeError);
}

TEST_CASE("qualified import syntax is rejected") {
	Noct::Runtime runtime {};
	Noct::Context ctx {};
	ctx.LoggingEnabled = false;

	auto r = runtime.RunString(ctx, "import math.sqrt");

	CHECK(r.HadParseError);
}

TEST_CASE("missing module function fails") {
	Noct::Runtime runtime {};
	runtime.registerModule("math")
	    .function("sqrt", [](double x) {
		    return std::sqrt(x);
	    });

	Noct::Context ctx {};
	ctx.LoggingEnabled = false;

	auto r = runtime.RunString(ctx, R"(
		import math
		math.foo(25);
	)");

	CHECK(r.HadRuntimeError);
}

TEST_CASE("fake engine module can be registered from C++") {
	Noct::Runtime runtime {};
	std::string spawned {};

	runtime.registerModule("engine")
	    .function("spawn_entity", [&](const std::string& name) {
		    spawned = name;
	    });

	Noct::Context ctx {};
	ctx.LoggingEnabled = false;
	auto r = runtime.RunString(ctx, R"(
		import engine
		engine.spawn_entity("Player");
	)");

	CHECK(!r.HadParseError);
	CHECK(!r.HadSemanticError);
	CHECK(!r.HadRuntimeError);
	CHECK(spawned == "Player");
}
