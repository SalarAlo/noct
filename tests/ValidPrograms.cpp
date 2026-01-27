#include <doctest/doctest.h>

#include "noct/Context.h"
#include "noct/Run.h"

using namespace Noct;

static void ExpectNoErrors(std::string_view src) {
	Context ctx {};
	ctx.LoggingEnabled = false;
	RunFromString(ctx, src);

	CHECK(!ctx.HadSemanticError);
	CHECK(!ctx.HadParseError);
	CHECK(!ctx.HadRuntimeError);
}

TEST_CASE("basic arithmetic") {
	ExpectNoErrors("print 1 + 2 * 3;");
}

TEST_CASE("variable declaration and use") {
	ExpectNoErrors("var x = 5; print x;");
}

TEST_CASE("block scoping") {
	ExpectNoErrors("var x = 1; { var y = x + 2; print y; }");
}

TEST_CASE("if statement") {
	ExpectNoErrors("if (true) { print 1; } else { print 2; }");
}

TEST_CASE("while loop") {
	ExpectNoErrors("var i = 0; while (i < 3) { i = i + 1; }");
}

TEST_CASE("function declaration and call") {
	ExpectNoErrors("fn add(a, b) { return a + b; } print add(2, 3);");
}

TEST_CASE("recursion") {
	ExpectNoErrors(
	    "fn fact(n) { if (n == 0) return 1; return n * fact(n-1); } print fact(5);");
}

TEST_CASE("lambda expression") {
	ExpectNoErrors("var f = fn(a) { return a * 2; }; print f(4);");
}

TEST_CASE("closure capture") {
	ExpectNoErrors(
	    "var x = 10; var f = fn() { return x; }; print f();");
}

TEST_CASE("nested scopes shadowing") {
	ExpectNoErrors(
	    "var x = 1; { var x = 2; print x; } print x;");
}

TEST_CASE("logical operators") {
	ExpectNoErrors("print true and false; print true or false;");
}

TEST_CASE("ternary operator") {
	ExpectNoErrors("print true ? 1 : 2;");
}

TEST_CASE("class declaration with method") {
	ExpectNoErrors(
	    "class A { fn f() { return 1; } }");
}

TEST_CASE("function inside class method") {
	ExpectNoErrors(
	    "class A { fn f() { var g = fn() { return 2; }; return g(); } }");
}

TEST_CASE("nested blocks and loops") {
	ExpectNoErrors(
	    "var i = 0; while (i < 2) { { var x = i; print x; } i = i + 1; }");
}
