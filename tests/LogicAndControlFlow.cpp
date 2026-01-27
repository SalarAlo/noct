
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

TEST_CASE("and short circuit") {
	CHECK(RunAndGet<bool>("false and (1 / 0);") == false);
}

TEST_CASE("or short circuit") {
	CHECK(RunAndGet<bool>("true or (1 / 0);") == true);
}

TEST_CASE("if true branch") {
	CHECK(RunAndGet<double>("if(true){1;} else {2;}") == 1.0);
}

TEST_CASE("if false branch") {
	CHECK(RunAndGet<double>("if(false){1;} else {2;}") == 2.0);
}

TEST_CASE("while loop increments") {
	CHECK(RunAndGet<double>("var i=0; while(i<3){ i=i+1; } i;") == 3.0);
}

TEST_CASE("nested loops") {
	CHECK(RunAndGet<double>(
	          "var i=0;"
	          "while(i<2){ var j=0; while(j<2){ j=j+1; } i=i+1; }"
	          "i;")
	    == 2.0);
}
