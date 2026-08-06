#!/usr/bin/env bash
set -euo pipefail

# Build ETP (Eden Treaty Pandemonium).
#
# Usage:
#   ./build.sh             Configure (if needed) and build
#   ./build.sh clean       Wipe build/ and do a full rebuild
#                          (needed after CMakeLists.txt changes, e.g. the
#                          app icon, the OUTPUT_NAME, or toggling wayland)
#   ./build.sh run         Build, then launch the result
#   ./build.sh wayland     Build raylib with native Wayland support (Linux)
#   ./build.sh clean wayland run
#
# Multiple arguments can be combined in any order.

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/build"

CLEAN=false
RUN=false
WAYLAND=false

for arg in "$@"; do
    case "$arg" in
        clean)   CLEAN=true ;;
        run)     RUN=true ;;
        wayland) WAYLAND=true ;;
        *)
            echo "Unknown argument: $arg"
            echo "Usage: $0 [clean] [run] [wayland]"
            exit 1
            ;;
    esac
done

if [[ "$CLEAN" == true && -d "$BUILD_DIR" ]]; then
    echo "Removing existing build directory..."
    rm -rf "$BUILD_DIR"
fi

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

CMAKE_ARGS=()
if [[ "$WAYLAND" == true ]]; then
    CMAKE_ARGS+=("-DETP_USE_WAYLAND=ON")
fi

echo "Configuring..."
cmake .. "${CMAKE_ARGS[@]}"

echo "Building..."
cmake --build . --parallel

if [[ -d "ETP.app" ]]; then
    RESULT="${BUILD_DIR}/ETP.app"
elif [[ -f "ETP" ]]; then
    RESULT="${BUILD_DIR}/ETP"
elif [[ -f "ETP.exe" ]]; then
    RESULT="${BUILD_DIR}/ETP.exe"
else
    echo "Build finished, but couldn't locate the ETP binary/app."
    exit 1
fi

echo "Build complete: ${RESULT}"

if [[ "$RUN" == true ]]; then
    if [[ -d "$RESULT" ]]; then
        open "$RESULT"
    else
        "$RESULT"
    fi
fi
