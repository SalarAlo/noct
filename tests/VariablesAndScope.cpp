#include <doctest/doctest.h>

#include "noct/Context.h"
#include "noct/Run.h"

template <typename T>
static T RunAndGet(std::string_view src) {
	Noct::Context ctx {};
	ctx.LoggingEnabled = false;
	Noct::RunResult r = RunFromString(ctx, src);
	REQUIRE(!r.HadParseError);
	REQUIRE(!r.HadSemanticError);
	REQUIRE(!r.HadRuntimeError);
	REQUIRE(std::holds_alternative<T>(r.Value));
	return std::get<T>(r.Value);
}

TEST_CASE("variable read") {
	CHECK(RunAndGet<double>("var x = 4; x;") == 4.0);
}

TEST_CASE("reassignment") {
	CHECK(RunAndGet<double>("var x = 1; x = x + 3; x;") == 4.0);
}

TEST_CASE("inner scope sees outer") {
	CHECK(RunAndGet<double>("var x = 2; { x; }") == 2.0);
}

TEST_CASE("shadowing does not leak") {
	CHECK(RunAndGet<double>("var x=1; { var x=5; } x;") == 1.0);
}

TEST_CASE("block scope destroyed after exit") {
	CHECK(RunAndGet<double>("var x=3; { var y=4; } x;") == 3.0);
}
