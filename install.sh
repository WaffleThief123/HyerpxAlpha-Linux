#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
INSTALL_DIR="/opt/hyperx"

echo "Installing HyperX Cloud Alpha Wireless manager..."

# Check for built binary
if [ ! -f "$SCRIPT_DIR/bin/Hyperx" ]; then
    echo "Error: bin/Hyperx not found. Build the project first:"
    echo "  cmake -S . -B build && cmake --build build"
    exit 1
fi

# Install binary and assets
sudo mkdir -p "$INSTALL_DIR/img"
sudo cp "$SCRIPT_DIR/bin/Hyperx" "$INSTALL_DIR/Hyperx"
sudo cp "$SCRIPT_DIR/bin/img/"*.png "$INSTALL_DIR/img/"
sudo chmod 755 "$INSTALL_DIR/Hyperx"

# Install udev rules
sudo cp "$SCRIPT_DIR/99-hyperx.rules" /etc/udev/rules.d/99-hyperx.rules
sudo udevadm control --reload-rules
sudo udevadm trigger

# Install autostart desktop entry
mkdir -p "$HOME/.config/autostart"
cp "$SCRIPT_DIR/hyperx-alpha.desktop" "$HOME/.config/autostart/hyperx-alpha.desktop"

# Also install to applications menu
sudo cp "$SCRIPT_DIR/hyperx-alpha.desktop" /usr/share/applications/hyperx-alpha.desktop

echo ""
echo "Installation complete!"
echo "  Binary:    $INSTALL_DIR/Hyperx"
echo "  udev:      /etc/udev/rules.d/99-hyperx.rules"
echo "  Autostart: ~/.config/autostart/hyperx-alpha.desktop"
echo ""
echo "Unplug and replug your headset dongle for udev rules to take effect."
echo "The app will start automatically on your next login, or run now with:"
echo "  /opt/hyperx/Hyperx --systray"
