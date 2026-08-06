#pragma once
#include "raylib.h"
#include <string>

// Builds an absolute path to a file under the assets/ folder that sits
// next to the running executable (Contents/MacOS/ inside a macOS .app
// bundle, or right beside the binary everywhere else). Using
// GetApplicationDirectory() instead of a bare relative path means asset
// loading works whether the game is launched from a terminal, double
// clicked in Finder, or run from a bundled .app.
inline std::string AssetPath(const char* relativePath) {
    return std::string(GetApplicationDirectory()) + "assets/" + relativePath;
}
