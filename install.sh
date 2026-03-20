#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

echo "Installing HyperX Cloud Alpha Wireless manager..."

# Check for built binary
if [ ! -f "$SCRIPT_DIR/bin/Hyperx" ]; then
    echo "Error: bin/Hyperx not found. Build the project first:"
    echo "  cmake -S . -B build && cmake --build build"
    exit 1
fi

# Install binary and assets
sudo install -Dm755 "$SCRIPT_DIR/bin/Hyperx" /usr/bin/Hyperx
sudo install -Dm644 -t /usr/share/hyperx/img/ "$SCRIPT_DIR/bin/img/"*.png

# Install udev rules
sudo install -Dm644 "$SCRIPT_DIR/99-hyperx.rules" /etc/udev/rules.d/99-hyperx.rules
sudo udevadm control --reload-rules
sudo udevadm trigger

# Install desktop entry and autostart
sudo install -Dm644 "$SCRIPT_DIR/hyperx-alpha.desktop" /usr/share/applications/hyperx-alpha.desktop
mkdir -p "$HOME/.config/autostart"
cp "$SCRIPT_DIR/hyperx-alpha.desktop" "$HOME/.config/autostart/hyperx-alpha.desktop"

echo ""
echo "Installation complete!"
echo "  Binary:    /usr/bin/Hyperx"
echo "  Assets:    /usr/share/hyperx/img/"
echo "  udev:      /etc/udev/rules.d/99-hyperx.rules"
echo "  Autostart: ~/.config/autostart/hyperx-alpha.desktop"
echo ""
echo "Unplug and replug your headset dongle for udev rules to take effect."
echo "The app will start automatically on your next login, or run now with:"
echo "  Hyperx --systray"
