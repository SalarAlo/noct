#include <doctest/doctest.h>

#include "noct/Context.h"
#include "noct/Run.h"

using namespace Noct;

static void ExpectSemanticError(std::string_view src) {
	Context ctx {};
	ctx.LoggingEnabled = false;
	RunFromString(ctx, src);

	CHECK(ctx.HadSemanticError);
	CHECK(!ctx.HadParseError);
}

TEST_CASE("duplicate variable in same scope") {
	ExpectSemanticError("var x = 1; var x = 2;");
}

TEST_CASE("use before declaration") {
	ExpectSemanticError("print x;");
}

TEST_CASE("break outside loop") {
	ExpectSemanticError("break;");
}

TEST_CASE("return outside function") {
	ExpectSemanticError("return 5;");
}

TEST_CASE("duplicate function parameter") {
	ExpectSemanticError("fn f(a, a) { print a; }");
}

TEST_CASE("variable in its own initializer") {
	ExpectSemanticError("var x = x;");
}

TEST_CASE("function declared twice in same scope") {
	ExpectSemanticError("fn a() {} fn a() {}");
}

TEST_CASE("parameter shadows parameter in lambda") {
	ExpectSemanticError("var f = fn (a, a) { return a; };");
}

TEST_CASE("this outside of class") {
	ExpectSemanticError(R"(
		fn f() { print this; }
	)");
}

TEST_CASE("this at top-level") {
	ExpectSemanticError(R"(
		print this;
	)");
}
