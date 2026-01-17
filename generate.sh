#!/bin/sh
set -e

BUILD_DIR=build

mkdir -p "$BUILD_DIR"
cmake -S . -B "$BUILD_DIR" -G Ninja
