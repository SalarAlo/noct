cmake_minimum_required(VERSION 3.20)

project(Noct VERSION 0.1.0 LANGUAGES CXX)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")

file(GLOB_RECURSE NOCT_LIB_SOURCES CONFIGURE_DEPENDS src/*.cpp)
list(REMOVE_ITEM NOCT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/Main.cpp")
list(FILTER NOCT_LIB_SOURCES EXCLUDE REGEX "/src/noct/")

find_package(fmt REQUIRED)

add_library(NoctLib ${NOCT_LIB_SOURCES})
add_library(Noct::Noct ALIAS NoctLib)
set_target_properties(NoctLib PROPERTIES EXPORT_NAME Noct)
target_include_directories(NoctLib
	PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
		$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
		$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/noct>
)
target_link_libraries(NoctLib PUBLIC fmt::fmt)

add_executable(Noct src/Main.cpp)
target_link_libraries(Noct PRIVATE NoctLib)

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
	tests/ClassesAndInstances.cpp
	tests/Modules.cpp
	tests/Embedding.cpp
)

target_include_directories(NoctTests PRIVATE vendor)
target_link_libraries(NoctTests PRIVATE NoctLib)

enable_testing()
add_test(NAME NoctTests COMMAND NoctTests)

install(TARGETS NoctLib
	EXPORT NoctTargets
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY src/
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/noct
	FILES_MATCHING
		PATTERN "*.h"
		PATTERN "src/noct/*" EXCLUDE
)

install(EXPORT NoctTargets
	FILE NoctTargets.cmake
	NAMESPACE Noct::
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Noct
)

configure_package_config_file(
	cmake/NoctConfig.cmake.in
	${CMAKE_CURRENT_BINARY_DIR}/NoctConfig.cmake
	INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Noct
)

write_basic_package_version_file(
	${CMAKE_CURRENT_BINARY_DIR}/NoctConfigVersion.cmake
	VERSION ${PROJECT_VERSION}
	COMPATIBILITY SameMajorVersion
)

install(FILES
	${CMAKE_CURRENT_BINARY_DIR}/NoctConfig.cmake
	${CMAKE_CURRENT_BINARY_DIR}/NoctConfigVersion.cmake
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Noct
)
