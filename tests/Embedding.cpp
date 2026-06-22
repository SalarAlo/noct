#include <doctest/doctest.h>

#include "Runtime.h"

TEST_CASE("embedding API extracts component declarations") {
	Noct::Runtime runtime {};
	runtime.register_native_module("Component");

	auto compile = runtime.compile_source(
	    Noct::ScriptId { "health-script" },
	    "scripts/health.noct",
	    R"(
			import Component
			Component.define("Health", "max", "Int", 100, "regen", "Float", 2.5, "enabled", "Bool", true);
		)");

	REQUIRE(compile);

	auto declarations = runtime.extract_component_declarations(compile.script());
	REQUIRE(declarations);
	REQUIRE(declarations.components().size() == 1);

	const auto& component = declarations.components()[0];
	CHECK(component.Name == "Health");
	CHECK(component.SourceScript.Value == "health-script");
	REQUIRE(component.Fields.size() == 3);
	CHECK(component.Fields[0].Name == "max");
	CHECK(component.Fields[0].Type == Noct::NoctFieldType::Int);
	CHECK(std::get<int64_t>(component.Fields[0].DefaultValue) == 100);
	CHECK(component.Fields[1].Name == "regen");
	CHECK(component.Fields[1].Type == Noct::NoctFieldType::Float);
	CHECK(std::get<double>(component.Fields[1].DefaultValue) == 2.5);
	CHECK(component.Fields[2].Name == "enabled");
	CHECK(component.Fields[2].Type == Noct::NoctFieldType::Bool);
	CHECK(std::get<bool>(component.Fields[2].DefaultValue));
}

TEST_CASE("embedding API reports invalid component declarations") {
	Noct::Runtime runtime {};
	runtime.register_native_module("Component");

	auto compile = runtime.compile_source(
	    Noct::ScriptId { "bad-script" },
	    "scripts/bad.noct",
	    R"(
			import Component
			Component.define("Broken", "name", "Int", "wrong");
		)");

	REQUIRE(compile);

	auto declarations = runtime.extract_component_declarations(compile.script());
	CHECK(!declarations);
	REQUIRE(!declarations.diagnostics().empty());
	CHECK(declarations.diagnostics()[0].Code == "component-default-type");
}

TEST_CASE("embedding API exposes dependency graph and invalidation") {
	Noct::Runtime runtime {};
	runtime.register_native_module("Component");
	runtime.register_native_module("engine");

	auto compile = runtime.compile_source(
	    Noct::ScriptId { "script-a" },
	    "scripts/a.noct",
	    R"(
			import Component
			import engine
		)");

	REQUIRE(compile);

	auto dependents = runtime.scripts_depending_on("engine");
	REQUIRE(dependents.size() == 1);
	CHECK(dependents[0] == "script-a");

	runtime.invalidate_script(Noct::ScriptId { "script-a" });
	CHECK(runtime.scripts_depending_on("engine").empty());
}

TEST_CASE("embedding API returns compile diagnostics") {
	Noct::Runtime runtime {};

	auto compile = runtime.compile_source(
	    Noct::ScriptId { "bad-parse" },
	    "scripts/bad_parse.noct",
	    "var = ;");

	CHECK(!compile);
	REQUIRE(!compile.diagnostics().empty());
	CHECK(compile.diagnostics()[0].Severity == Noct::NoctSeverity::Error);
	CHECK(compile.diagnostics()[0].Location.File == "scripts/bad_parse.noct");
}
