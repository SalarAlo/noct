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

TEST_CASE("construct instance") {
	CHECK(RunAndGet<std::shared_ptr<Noct::ClassInstance>>(R"(
		class A {}
		A();
	)") != nullptr);
}

TEST_CASE("set and get field") {
	CHECK(RunAndGet<double>(R"(
		class A {}
		var a = A();
		a.x = 5;
		a.x;
	)") == 5.0);
}

TEST_CASE("instances do not share fields") {
	CHECK(RunAndGet<double>(R"(
		class A {}
		var a = A();
		var b = A();
		a.x = 1;
		b.x = 2;
		a.x + b.x;
	)") == 3.0);
}

TEST_CASE("method reads and writes this") {
	CHECK(RunAndGet<double>(R"(
		class Flyer {
			fn fly() {
				this.x = 2;
				return this.x;
			}
		}

		var x = Flyer();
		x.fly();
	)") == 2.0);
}

TEST_CASE("method mutation persists on instance") {
	CHECK(RunAndGet<double>(R"(
		class Flyer {
			fn setX(v) { this.x = v; }
			fn getX() { return this.x; }
		}

		var x = Flyer();
		x.setX(42);
		x.getX();
	)") == 42.0);
}

TEST_CASE("this works inside nested function in method") {
	CHECK(RunAndGet<double>(R"(
		class Flyer {
			fn test() {
				var f = fn() { this.x = 7; };
				f();
			}
		}

		var x = Flyer();
		x.test();
		x.x;
	)") == 7.0);
}

TEST_CASE("retrieving bound method keeps correct this") {
	CHECK(RunAndGet<double>(R"(
		class A {
			fn set(v) { this.x = v; }
		}

		var a = A();
		var s = a.set;
		s(123);
		a.x;
	)") == 123.0);
}

TEST_CASE("method can capture outer variables") {
	CHECK(RunAndGet<double>(R"(
		var y = 10;

		class A {
			fn f() { return y; }
		}

		var a = A();
		a.f();
	)") == 10.0);
}

TEST_CASE("this outside class is error") {
	Noct::Context ctx {};
	ctx.LoggingEnabled = false;

	auto r = RunFromString(ctx, R"(
		this;
	)");

	CHECK((r.HadSemanticError || r.HadRuntimeError));
}

TEST_CASE("property access on non instance fails") {
	Noct::Context ctx {};
	ctx.LoggingEnabled = false;

	auto r = RunFromString(ctx, R"(
		var x = 5;
		x.y;
	)");

	CHECK(r.HadRuntimeError);
}

TEST_CASE("setting field on non instance fails") {
	Noct::Context ctx {};
	ctx.LoggingEnabled = false;

	auto r = RunFromString(ctx, R"(
		var x = "hi";
		x.y = 3;
	)");

	CHECK(r.HadRuntimeError);
}

TEST_CASE("calling method on class instead of instance fails") {
	Noct::Context ctx {};
	ctx.LoggingEnabled = false;

	auto r = RunFromString(ctx, R"(
		class A { fn f() {} }
		A.f();
	)");

	CHECK(r.HadRuntimeError);
}

TEST_CASE("bound method stays bound to original instance") {
	CHECK(RunAndGet<double>(R"(
		class A { fn set(v) { this.x = v; } }

		var a = A();
		var b = A();

		var f = a.set;
		f(10);

		b.set(20);

		a.x + b.x;
	)") == 30.0);
}

TEST_CASE("methods on different instances keep separate this") {
	CHECK(RunAndGet<double>(R"(
		class A { fn set(v) { this.x = v; } }

		var a = A();
		var b = A();

		a.set(1);
		b.set(2);

		a.x * 10 + b.x;
	)") == 12.0);
}

TEST_CASE("init method runs as constructor") {
	CHECK(RunAndGet<double>(R"(
		class A {
			fn init(v) { this.x = v; }
		}

		var a = A(55);
		a.x;
	)") == 55.0);
}

TEST_CASE("constructor arity enforced") {
	Noct::Context ctx {};
	ctx.LoggingEnabled = false;

	auto r = RunFromString(ctx, R"(
		class A { fn init(x) {} }
		A();
	)");

	CHECK(r.HadRuntimeError);
}

TEST_CASE("method can return this") {
	CHECK(RunAndGet<std::shared_ptr<Noct::ClassInstance>>(R"(
		class A {
			fn me() { return this; }
		}
		A().me();
	)") != nullptr);
}

TEST_CASE("deep nested closures share correct this") {
	CHECK(RunAndGet<double>(R"(
		class A {
			fn f() {
				var g = fn() {
					var h = fn() { this.x = 9; };
					h();
				};
				g();
			}
		}

		var a = A();
		a.f();
		a.x;
	)") == 9.0);
}

TEST_CASE("calling missing method fails") {
	Noct::Context ctx {};
	ctx.LoggingEnabled = false;

	auto r = RunFromString(ctx, R"(
		class A {}
		A().nope();
	)");

	CHECK(r.HadRuntimeError);
}
