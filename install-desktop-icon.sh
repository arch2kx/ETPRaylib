#!/usr/bin/env bash
set -euo pipefail

# Install a .desktop launcher for ETP (Linux only).
#
# By default installs into the application menu (~/.local/share/applications).
# Pass "desktop" to also (or instead) place an icon on your actual Desktop
# folder - on GNOME/Nautilus this additionally marks it "trusted" so it's
# double-clickable without an "Untrusted application launcher" warning.
#
# Usage:
#   ./install-desktop-icon.sh                 Install app-menu entry
#   ./install-desktop-icon.sh desktop          Also install a Desktop icon
#   ./install-desktop-icon.sh remove           Remove the app-menu entry
#   ./install-desktop-icon.sh remove desktop   Also remove the Desktop icon

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/build"
BINARY="${BUILD_DIR}/ETP"
ICON="${SCRIPT_DIR}/assets/etp_cpp.png"

MENU_DIR="${HOME}/.local/share/applications"
MENU_FILE="${MENU_DIR}/etp.desktop"

# Respect the user's actual (possibly localized/relocated) Desktop folder
if command -v xdg-user-dir >/dev/null 2>&1; then
    DESKTOP_DIR="$(xdg-user-dir DESKTOP)"
else
    DESKTOP_DIR="${HOME}/Desktop"
fi
DESKTOP_ICON_FILE="${DESKTOP_DIR}/etp.desktop"

write_desktop_entry() {
    local target_file="$1"
    cat > "$target_file" <<EOF
[Desktop Entry]
Type=Application
Name=Eden Treaty Pandemonium
Comment=Bullet-hell shoot-em-up
Exec=${BINARY}
Icon=${ICON}
Terminal=false
Categories=Game;ActionGame;
EOF
    chmod +x "$target_file"
}

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

if [[ ! -x "$BINARY" ]]; then
    echo "Couldn't find a built binary at ${BINARY}."
    echo "Build it first with: ./build.sh"
    exit 1
fi

if [[ ! -f "$ICON" ]]; then
    echo "Couldn't find icon at ${ICON}."
    exit 1
fi

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
