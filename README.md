# Apple Silicon to Apple Silicon VDM tool

This tool lets you send common Apple VDM messages from an Apple Sillicon Mac.  It can send messages to the T2 DFU port, the Apple M1 DFU port or any USB-C based iPad.  Currently it requires the sending device to be M1 based and the "DFU" port, but it should be possible to use any port and work from an Intel Mac (they have the same USB-C port controller) with some additonal patching.

`dfu` and `reboot` are confirmed to work on iPad and T2.  Serial can probably be adapted to work with checkra1n.

## Disclaimer

I have no idea what I'm doing with IOKit and CoreFoundation -marcan

## Copyright

This is based on portions of [ThunderboltPatcher](https://github.com/osy/ThunderboltPatcher) and licensed under Apache-2.0.

* Copyright (C) 2019 osy86. All rights reserved.
* Copyright (C) 2021 The Asahi Linux Contributors

Thanks to t8012.dev and mrarm for assistance with the VDM and Ace2 host interface commands.

The documentation of the ACE and its USB-PD VPDs is here https://blog.t8012.dev/ace-part-1/

## Building

Install the XCode commandline tools and type `make`.

## Usage

Connect the two devices via their DFU ports. That's the furthest left port on M1 MacBooks, the closest left port on the T2, and the only port on the iPad.  See Apple's DFU recovery support articles to identify these ports on other models.

You need to use a *USB 3.0 compatible* (SuperSpeed) Type C cable. USB 2.0-only cables, including most cables meant for charging, will not work, as they do not have the required pins (USB CC1/CC2 where USB-PD are transmitted). Thunderbolt cables work too.

Run it as root (`sudo ./macvdmtool`) as root privledge is required to open the needed IOKit IOService.

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
