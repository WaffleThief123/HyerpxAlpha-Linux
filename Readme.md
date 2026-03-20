# HyerpxAlpha

Linux software for the HyperX Cloud Alpha Wireless headset.

![HyerpxAlpha](assets/Hyperx.png)

## Features

- Battery monitoring (percentage and estimated hours remaining)
- Sleep timer (10, 20, or 30 minutes)
- Microphone monitor (sidetone)
- Voice prompts toggle
- System tray support with battery level icons
- Settings persist across restarts

## Dependencies

- wxWidgets 3.2+
- hidapi

**Arch Linux:**
```
sudo pacman -S wxwidgets-gtk3 hidapi
```

**Ubuntu/Debian:**
```
sudo apt install libwxgtk3.2-dev wx3.2-headers libhidapi-dev libhidapi-libusb0 libhidapi-hidraw0
```

## Building

```
git clone https://github.com/WaffleThief123/HyerpxAlpha-Linux
cd HyerpxAlpha-Linux
cmake -S . -B build
cmake --build build
```

## Installation

An install script is included that sets up the binary, udev rules, and autostart entry:

```
./install.sh
```

This will:
- Copy the binary and icons to `/opt/hyperx/`
- Install udev rules for non-root device access
- Add a desktop entry to your applications menu
- Set up autostart so the tray app launches at login

After installing, unplug and replug your headset dongle for the udev rules to take effect.

### Manual Setup

If you prefer to set things up yourself:

**udev rules** (required for non-root access):
```
sudo cp 99-hyperx.rules /etc/udev/rules.d/
sudo udevadm control --reload-rules
sudo udevadm trigger
```

**Run directly:**
```
bin/Hyperx
```

**Run as a system tray app:**
```
bin/Hyperx --systray
```

Note: System tray mode requires legacy tray support (e.g., the system tray in KDE, XFCE, or a tray applet on GNOME).

## Contributing

Contributions are welcome! Please fork this repository and submit pull requests.

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.
