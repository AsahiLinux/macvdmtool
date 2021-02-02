# Apple Silicon to Apple Silicon VDM tool

This tool lets you get a serial console on an Apple Silicon device and reboot it remotely, using only another Apple Silicon device running macOS and a standard Type C cable.

## Disclaimer

I have no idea what I'm doing with IOKit and CoreFoundation -marcan

## Copyright

This is based on portions of [ThunderboltPatcher](https://github.com/osy/ThunderboltPatcher) and licensed under Apache-2.0.

* Copyright (C) 2019 osy86. All rights reserved.
* Copyright (C) 2021 The Asahi Linux Contributors

## Building

Install the XCode commandline tools and type `make`.

## Usage

Run it as root (`sudo ./macvdmtool`).

```
Usage: ./macvdmtool <command>
Commands:
  serial - enter serial mode on both ends
  reboot - reboot the target
  reboot serial - reboot the target and enter serial mode
  dfu - put the target into DFU mode
  nop - do nothing
```

Use `/dev/cu.debug_console` on the local machine as your serial device. To use it with m1n1, `export M1N1DEVICE=/dev/cu.debug-console`.

For typical development, the command you want to use is `macvdmtool reboot serial`. This will reboot the target, and immediately put it back into serial mode, with the right timing to make it work.
