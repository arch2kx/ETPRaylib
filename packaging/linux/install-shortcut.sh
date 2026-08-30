#!/usr/bin/env bash
set -euo pipefail

# Install a .desktop launcher for ETP (Linux only).
#
# This script serves two situations with the same code:
#   - inside an extracted download, where ETP sits next to this script
#   - inside a source checkout, where the binary is at build/ETP
# It looks for the former first and falls back to the latter, so the repo
# root copy (a symlink to this file) and the shipped copy behave identically.
#
# By default installs into the application menu (~/.local/share/applications).
# Pass "desktop" to also place an icon on your actual Desktop folder - on
# GNOME/Nautilus this additionally marks it "trusted" so it's double-clickable
# without an "Untrusted application launcher" warning.
#
# Usage:
#   ./install-shortcut.sh                 Install app-menu entry
#   ./install-shortcut.sh desktop         Also install a Desktop icon
#   ./install-shortcut.sh remove          Remove the app-menu entry
#   ./install-shortcut.sh remove desktop  Also remove the Desktop icon

# Deliberately NOT resolved with readlink -f: the repo root copy is a symlink
# into packaging/linux/, and resolving it would make the script look for
# build/ETP inside packaging/linux/ instead of the checkout root.
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

MENU_DIR="${HOME}/.local/share/applications"
MENU_FILE="${MENU_DIR}/ETP.desktop"

# Respect the user's actual (possibly localized/relocated) Desktop folder
if command -v xdg-user-dir >/dev/null 2>&1; then
    DESKTOP_DIR="$(xdg-user-dir DESKTOP)"
else
    DESKTOP_DIR="${HOME}/Desktop"
fi
DESKTOP_ICON_FILE="${DESKTOP_DIR}/ETP.desktop"

# --- Removal doesn't need a binary, so handle it before locating one ----
if [[ "${1:-}" == "remove" ]]; then
    rm -f "$MENU_FILE"
    echo "Removed ${MENU_FILE}"
    if command -v update-desktop-database >/dev/null 2>&1; then
        update-desktop-database "$MENU_DIR" >/dev/null 2>&1 || true
    fi
    if [[ "${2:-}" == "desktop" ]]; then
        rm -f "$DESKTOP_ICON_FILE"
        echo "Removed ${DESKTOP_ICON_FILE}"
    fi
    exit 0
fi

# --- Locate the game: extracted download first, source checkout second --
if [[ -x "${SCRIPT_DIR}/ETP" ]]; then
    GAME_DIR="${SCRIPT_DIR}"
elif [[ -x "${SCRIPT_DIR}/build/ETP" ]]; then
    GAME_DIR="${SCRIPT_DIR}/build"
else
    echo "Couldn't find the ETP binary next to this script or in build/."
    echo "If you're in a source checkout, build it first with: ./build.sh"
    exit 1
fi

# Prefer the wrapper - it fixes the working directory so assets/ resolves.
if [[ -x "${GAME_DIR}/play.sh" ]]; then
    EXEC_TARGET="${GAME_DIR}/play.sh"
else
    EXEC_TARGET="${GAME_DIR}/ETP"
fi

# Packaged downloads ship icon.png at the root; checkouts have the original.
if [[ -f "${GAME_DIR}/icon.png" ]]; then
    ICON="${GAME_DIR}/icon.png"
elif [[ -f "${SCRIPT_DIR}/assets/mikaIcon.png" ]]; then
    ICON="${SCRIPT_DIR}/assets/mikaIcon.png"
elif [[ -f "${GAME_DIR}/assets/mikaIcon.png" ]]; then
    ICON="${GAME_DIR}/assets/mikaIcon.png"
else
    echo "Couldn't find an icon (icon.png or assets/mikaIcon.png)."
    exit 1
fi

write_desktop_entry() {
    local target_file="$1"
    cat > "$target_file" <<EOF
[Desktop Entry]
Type=Application
Name=Eden Treaty Pandemonium
Comment=Bullet-hell shoot-em-up
Exec=${EXEC_TARGET}
Icon=${ICON}
Path=${GAME_DIR}
Terminal=false
Categories=Game;ActionGame;
EOF
    chmod +x "$target_file"
}

mkdir -p "$MENU_DIR"
write_desktop_entry "$MENU_FILE"
echo "Installed app-menu entry: ${MENU_FILE}"

if command -v update-desktop-database >/dev/null 2>&1; then
    update-desktop-database "$MENU_DIR" >/dev/null 2>&1 || true
fi

if [[ "${1:-}" == "desktop" ]]; then
    mkdir -p "$DESKTOP_DIR"
    write_desktop_entry "$DESKTOP_ICON_FILE"
    echo "Installed Desktop icon: ${DESKTOP_ICON_FILE}"

    # GNOME/Nautilus refuses to run untrusted launchers from the Desktop
    # until they're explicitly marked trusted.
    if command -v gio >/dev/null 2>&1; then
        gio set "$DESKTOP_ICON_FILE" metadata::trusted true >/dev/null 2>&1 || true
    fi
fi

echo "Done. It may take a moment (or a logout/login) to show up."
