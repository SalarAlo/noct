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

TEST_CASE("addition") {
	CHECK(RunAndGet<double>("1 + 2;") == 3.0);
}

TEST_CASE("operator precedence") {
	CHECK(RunAndGet<double>("1 + 2 * 3;") == 7.0);
}

TEST_CASE("parentheses override precedence") {
	CHECK(RunAndGet<double>("(1 + 2) * 3;") == 9.0);
}

TEST_CASE("unary minus") {
	CHECK(RunAndGet<double>("-5;") == -5.0);
}

TEST_CASE("division") {
	CHECK(RunAndGet<double>("8 / 2;") == 4.0);
}

TEST_CASE("mixed operations") {
	CHECK(RunAndGet<double>("10 - 2 * 3 + 4;") == 8.0);
}

TEST_CASE("nested parentheses") {
	CHECK(RunAndGet<double>("((2 + 3) * (4 - 1));") == 15.0);
}

TEST_CASE("floating point") {
	CHECK(RunAndGet<double>("5.5 + 1.5;") == 7.0);
}
