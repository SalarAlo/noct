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

TEST_CASE("read undefined property") {
	ExpectRuntimeError(R"(
		class A {}
		var a = A();
		print a.missing;
	)");
}

TEST_CASE("set instance method is forbidden") {
	ExpectRuntimeError(R"(
		class A { fn f() { return 1; } }
		var a = A();
		a.f = 3;
	)");
}

TEST_CASE("only instances can have properties - get on non-instance") {
	ExpectRuntimeError(R"(
		var x = 123;
		print x.y;
	)");
}

TEST_CASE("only instances can have properties - set on non-instance") {
	ExpectRuntimeError(R"(
		var x = 123;
		x.y = 7;
	)");
}

TEST_CASE("calling unbound method from class is not allowed") {
	ExpectRuntimeError(R"(
		class A { fn f() { return 1; } }
		var c = A;      // class value
		c.f();          // should fail: only instances can have properties / call
	)");
}
