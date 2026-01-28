cmake_minimum_required(VERSION 3.20)

project(Noct LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")

file(GLOB_RECURSE NOCT_LIB_SOURCES CONFIGURE_DEPENDS src/noct/*.cpp)

find_package(fmt REQUIRED)

add_library(NoctLib ${NOCT_LIB_SOURCES})
target_include_directories(NoctLib PUBLIC src)
target_link_libraries(NoctLib PUBLIC fmt::fmt)

add_executable(Noct src/Main.cpp)
target_link_libraries(Noct PRIVATE NoctLib)

# ---------- tests ----------
add_executable(NoctTests 
	tests/SyntaxErrors.cpp
	tests/SemanticErrors.cpp
	tests/RuntimeErrors.cpp
	tests/ValidPrograms.cpp
	tests/Arithmetic.cpp
	tests/Closures.cpp
	tests/Functions.cpp
	tests/LogicAndControlFlow.cpp
	tests/VariablesAndScope.cpp
)
target_include_directories(NoctTests PRIVATE vendor)
target_link_libraries(NoctTests PRIVATE NoctLib)

enable_testing()
add_test(NAME NoctTests COMMAND NoctTests)
