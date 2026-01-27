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

TEST_CASE("simple function return") {
	CHECK(RunAndGet<double>("fn f(){ return 5; } f();") == 5.0);
}

TEST_CASE("function parameters") {
	CHECK(RunAndGet<double>("fn add(a,b){ return a+b; } add(2,3);") == 5.0);
}

TEST_CASE("recursive call") {
	CHECK(RunAndGet<double>(
	          "fn fact(n){ if(n==0) return 1; return n * fact(n-1); } fact(4);")
	    == 24.0);
}

TEST_CASE("multiple returns") {
	CHECK(RunAndGet<double>(
	          "fn f(x){ if(x) return 1; return 2; } f(false);")
	    == 2.0);
}
