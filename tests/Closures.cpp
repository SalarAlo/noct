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

TEST_CASE("closure captures outer variable") {
	CHECK(RunAndGet<double>("var x = 7; var f = fn(){ return x; }; f();") == 7.0);
}

TEST_CASE("closure survives scope exit") {
	CHECK(RunAndGet<double>("var f; { var x = 9; f = fn(){ return x; }; } f();") == 9.0);
}

TEST_CASE("multiple closures share state") {
	CHECK(RunAndGet<double>(
	          "var x = 1;"
	          "var inc = fn(){ x = x + 1; };"
	          "inc(); inc();"
	          "x;")
	    == 3.0);
}
