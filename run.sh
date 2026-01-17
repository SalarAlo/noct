
#!/bin/sh
set -e

BUILD_DIR=build
BIN="$BUILD_DIR/Noct"

cmake --build "$BUILD_DIR"

exec "$BIN" "$@"
