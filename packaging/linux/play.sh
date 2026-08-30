#!/usr/bin/env bash
set -euo pipefail

# Launch ETP with the working directory set to the game folder.
#
# The game loads assets/ via relative paths, so running the binary directly
# from somewhere else (a file manager, a menu entry, another terminal) would
# fail to find them. This wrapper makes the launch location irrelevant.

cd "$(dirname "$(readlink -f "${BASH_SOURCE[0]}")")"
exec ./ETP "$@"
