cmake_minimum_required(VERSION 3.20)

project(Noct LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB_RECURSE NOCT_SOURCES
     CONFIGURE_DEPENDS
     src/*.cpp
)

find_package(fmt REQUIRED)


add_executable(Noct ${NOCT_SOURCES})

target_include_directories(Noct
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(Noct PRIVATE fmt::fmt)
