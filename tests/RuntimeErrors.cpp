#include <doctest/doctest.h>

#include "noct/Context.h"
#include "noct/Run.h"

using namespace Noct;

static void ExpectRuntimeError(std::string_view src) {
	Context ctx {};
	ctx.LoggingEnabled = false;
	RunFromString(ctx, src);

	CHECK(ctx.HadRuntimeError);
	CHECK(!ctx.HadParseError);
	CHECK(!ctx.HadSemanticError);
}

TEST_CASE("divide by zero") {
	ExpectRuntimeError("print 4 / 0;");
}

TEST_CASE("calling non-function value") {
	ExpectRuntimeError("var x = 5; x();");
}

TEST_CASE("add number and function") {
	ExpectRuntimeError("print 1 + fn(){};");
}

TEST_CASE("read variable before assignment") {
	ExpectRuntimeError("var x; print x;");
}

TEST_CASE("function called with wrong number of args") {
	ExpectRuntimeError("fn f(a, b) { return a; } f(1);");
}
