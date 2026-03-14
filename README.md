# Apple Silicon to Apple Silicon VDM tool

This tool lets you get a serial console on an Apple Silicon device and reboot it remotely, using only another Apple Silicon device running macOS and a standard Type C cable.

## Disclaimer

I have no idea what I'm doing with IOKit and CoreFoundation -marcan

## Copyright

This is based on portions of [ThunderboltPatcher](https://github.com/osy/ThunderboltPatcher) and licensed under Apache-2.0.

* Copyright (C) 2019 osy86. All rights reserved.
* Copyright (C) 2021 The Asahi Linux Contributors

Thanks to t8012.dev and mrarm for assistance with the VDM and Ace2 host interface commands.

## Note about macOS 12 and 13

Access to the serial console device on macOS Monterey (12) and Ventura (13) was blocked by the `AppleSerialShim` extension. Update to later macOS version or look into the history of this document to generate a kernelcache without the `AppleSerialShim` extension.

## Building

Install the XCode commandline tools and type `make`.

## Usage

Connect the two devices via their DFU ports. That's:
 - the rear port on MacBook Air and 13" MacBook Pro
 - the port next to the MagSafe connector on the 14" and 16" MacBook Pro
 - the port nearest to the power plug on Mac Mini

For debugusb mode USB 2.0-only cables like Apple includes for non-Magsafe Macbooks do work. For serial mode a *USB 3.0 compatible* (SuperSpeed) Type C cable. USB 2.0-only cables, including most cables meant for charging, will not work, as they do not have the required pins. Thunderbolt cables work too.

Run it as root (`sudo ./macvdmtool`).

```
Usage: ./macvdmtool <command>
Commands:
  serial - enter serial mode on both ends
  debugusb - enter debug usb mode on the target
  reboot - reboot the target
  reboot serial - reboot the target and enter serial mode
  reboot debugusb - reboot the target and enter debugusb mod
  dfu - put the target into DFU mode
  nop - do nothing
```

For serial mode use `/dev/cu.debug_console` on the local machine as your serial device. To use it with m1n1, `export M1N1DEVICE=/dev/cu.debug-console`.<br>
For debugusb mode use `/dev/cu.kis-100000-ch-0` on the local machine as your serial device. To use it with m1n1, `export M1N1DEVICE=/dev/cu.kis-100000-ch-0`.

For typical development, the command you want to use is `macvdmtool reboot serial` or `macvdmtool reboot debugusb`. This will reboot the target, and immediately put it back into serial or debugusb mode, with the right timing to make it work.
