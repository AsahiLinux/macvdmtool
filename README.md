# Apple Silicon to Apple Silicon VDM tool

This tool lets you get a serial console on an Apple Silicon device and reboot it remotely, using only another Apple Silicon device running macOS and a standard Type C cable.

## Disclaimer

I have no idea what I'm doing with IOKit and CoreFoundation -marcan

## Copyright

This is based on portions of [ThunderboltPatcher](https://github.com/osy/ThunderboltPatcher) and licensed under Apache-2.0.

* Copyright (C) 2019 osy86. All rights reserved.
* Copyright (C) 2021 The Asahi Linux Contributors

Thanks to t8012.dev and mrarm for assistance with the VDM and Ace2 host interface commands.

## Note about macOS 12

To have access to the serial console device on macOS Monterey (12), you need to disable the `AppleSerialShim` extension.

**Note:** This requires downgrading the system security and may cause problems with upgrades. Use it at your own risk!

Start by generating a new kernel cache, without the `AppleSerialShim` extension:

```
sudo kmutil create -n boot -a arm64e -B /Library/KernelCollections/kc.noshim.macho -V release  -k /System/Library/Kernels/kernel.release.<soc> -r /System/Library/Extensions -r /System/Library/DriverExtensions -x $(kmutil inspect -V release --no-header | awk '!/AppleSerialShim/ { print " -b "$1; }')
```

Replace `<soc>` with `t8101` on M1 Macs and `t6000` on M1 Pro/Max Macs. If you’re unsure, `uname -v` and look at the end of the version string (`RELEASE_ARM64_<soc>`).

If you receive an error while generating the new kernel cache, "Operation not permitted," then you will need to disable SIP before executing the above step. SIP can be disabled by entering the 1TR with the steps described below, but instead issuing:

```
csrutil disable
```

At the 1TR terminal.

Once you've generated a new kernel cache, enter 1TR:

1. Power off your Mac
2. Press and hold the Power button until the boot menu appears
3. Select “Options”, then (if necessary) select your macOS volume and enter your administrative password.

Select Utilities>Startup security and switch the macOS installation to reduced security. Exit Startup security.

Select Utilities>Terminal and install your custom kernel:

```
kmutil configure-boot -c /Volume/<volume>/Library/KernelCollections/kc.noshim.macho -C -v /Volume/<volume>
```

Replace `<volume>` with the name of your boot volume.

You can now reboot: macOS should start as normal, and the serial device `/dev/cu.debug-console` should be available.

To revert back to the default kernel, enter 1TR again, access Utilities>Startup security and switch to full or reduced security.

## Building

Install the XCode commandline tools and type `make`.

## Usage

Connect the two devices via their DFU ports. That's:
 - the rear port on MacBook Air and 13" MacBook Pro
 - the port next to the MagSafe connector on the 14" and 16" MacBook Pro
 - the port nearest to the power plug on Mac Mini
 - the port nearest to the ethernet port on the Mac Studio

You need to use a *USB 3.0 compatible* (SuperSpeed) Type C cable. USB 2.0-only cables, including most cables meant for charging, will not work, as they do not have the required pins. Thunderbolt cables work too.

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
