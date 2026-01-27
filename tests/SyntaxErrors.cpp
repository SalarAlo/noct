#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "noct/Context.h"
#include "noct/Run.h"

using namespace Noct;

static void ExpectParseError(std::string_view src) {
	Context ctx {};
	ctx.LoggingEnabled = false;
	RunFromString(ctx, src);
	CHECK(ctx.HadParseError);
	CHECK(!ctx.HadRuntimeError);
}

TEST_CASE("unterminated string") {
	ExpectParseError("print \"Hello;");
}

TEST_CASE("missing semicolon after statement") {
	ExpectParseError("print 1");
}

TEST_CASE("missing semicolon after var declaration") {
	ExpectParseError("var x = 0");
}

TEST_CASE("unclosed block") {
	ExpectParseError("{ print 1;");
}

TEST_CASE("unexpected token in expression") {
	ExpectParseError("print + ;");
}

TEST_CASE("invalid assignment target") {
	ExpectParseError("(1 + 2) = 3;");
}

TEST_CASE("missing closing parenthesis") {
	ExpectParseError("print (1 + 2;");
}

TEST_CASE("extra closing parenthesis") {
	ExpectParseError("print 1 + 2);");
}

TEST_CASE("if without condition") {
	ExpectParseError("if { print 1; }");
}

TEST_CASE("while without condition") {
	ExpectParseError("while { print 1; }");
}

TEST_CASE("function missing parameter list") {
	ExpectParseError("fn test { print 1; }");
}

TEST_CASE("function missing body") {
	ExpectParseError("fn test();");
}

TEST_CASE("lambda missing body") {
	ExpectParseError("var f = fn (a, b) ;");
}

TEST_CASE("ternary missing branch") {
	ExpectParseError("print true ? 1 : ;");
}

TEST_CASE("nested parentheses mismatch") {
	ExpectParseError("print ((1 + 2);");
}

TEST_CASE("missing comma in call") {
	ExpectParseError("f(1 2);");
}

TEST_CASE("trailing comma in params") {
	ExpectParseError("fn f(a, ) {}");
}

TEST_CASE("empty ternary") {
	ExpectParseError("print ? 1 : 2;");
}

TEST_CASE("block where expression expected") {
	ExpectParseError("print { 1; };");
}
