[![Hits](https://hits.seeyoufarm.com/api/count/incr/badge.svg?url=https%3A%2F%2Fgithub.com%2Fpine64%2Fblisp&count_bg=%235791AC&title_bg=%23555555&icon=airplayaudio.svg&icon_color=%23D2D9DD&title=hits&edge_flat=false)](https://github.com/pine64/blisp/wiki/Update-Pinecil-V2)
[![GitHub all downloads](https://img.shields.io/github/downloads/pine64/blisp/total?color=5791ac&logo=docusign&logoColor=white)](https://github.com/pine64/blisp/releases/tag/v0.0.4)
[![Discord](https://img.shields.io/discord/463237927984693259?color=5791ac&logo=discord&logoColor=white)](https://discord.com/invite/pine64)
[![GitHub release](https://img.shields.io/github/v/release/pine64/blisp?color=5791ac)](https://github.com/pine64/blisp/releases/tag/v0.0.4)

<img src="./img/Gradient-white-blue-03.png" align="left" width="60" > <br clear="left" />
# BLISP 

Bouffalo Labs ISP (in-system-programming) tool & library: an open source tool to flash Bouffalo RISC-V MCUs.

**NOTE:** Library API and `blisp` tool cli arguments are not stable yet.
<br>

## Supported MCUs
- [x] `bl60x` - BL602 / BL604 / TG7100C / LF686 / LF688
- [x] `bl70x` - BL702 / BL704 / BL706
<br>

## Supported Devices 
 | System  | <img width="12" src="img/win32.png" /> Windows | <img width="15" src="https://cdn.simpleicons.org/Apple/5791ac" /> MacOS| <img width="17" src="https://cdn.simpleicons.org/Linux/5791ac" /> Linux| <img width="15" src="https://cdn.simpleicons.org/Freebsd/5791ac" /> FreeBSD |
 | :-----: | :------: | :------: | :------: | :------: |
 | Pinecil V2  |<img width="22" src="https://cdn.simpleicons.org/cachet/5791ac" />|<img width="22" src="https://cdn.simpleicons.org/cachet/5791ac" />| <img width="22" src="https://cdn.simpleicons.org/cachet/5791ac" />| <img width="22" src="https://cdn.simpleicons.org/cachet/5791ac" /> |
 | Pinecone  |<img width="22" src="https://cdn.simpleicons.org/cachet/5791ac" />|<img width="22" src="https://cdn.simpleicons.org/cachet/5791ac" />|<img width="22" src="https://cdn.simpleicons.org/cachet/5791ac" />| <img width="22" src="https://cdn.simpleicons.org/cachet/5791ac" /> |
 <br>
 
## How to update Pinecil V2

Download the newest release of [Blisp updater here](https://github.com/pine64/blisp/releases/).

Check out the [wiki page](https://github.com/pine64/blisp/wiki/Update-Pinecil-V2) for install instructions.
<br><br>

## Building from code

### Clone repository

If you have not cloned this repository locally; clone the git repository locally by running

```bash
git clone --recursive https://github.com/pine64/blisp.git
cd blisp
git submodule update --init --recursive
```

If vendor/argtable3 and vendor/libserialport/ are empty, this last step has
failed and should be investigated.

### Build the library and command line utility

For building `blisp` command line tool, use following commands:

```bash
mkdir build && cd build
cmake -DBLISP_BUILD_CLI=ON ..
cmake --build .
```

For building against preinstalled system libraries of the used vendor
libraries (e.g. for use by system maintainers), additionally define
`BLISP_USE_SYSTEM_LIBRARIES`, e.g. using following commands:
```bash
mkdir build && cd build
cmake -DBLISP_USE_SYSTEM_LIBRARIES=ON -DBLISP_BUILD_CLI=ON ..
cmake --build .
```

#### Need more build details? [See here](https://github.com/pine64/blisp/wiki/Update-Pinecil-V2#build-blisp-flasher-from-code).

## Usage

For BL70X, BL61X, BL808 and BL606P, connected via USB, you can use following command, which will auto-detect serial port on Windows:

```bash
.\blisp.exe write --chip=bl70x --reset .\name_of_firmware.bin
or
.\blisp.exe write -c bl70x --reset .\name_of_firmware.bin
```

For BL60X, you need to specify also the serial port path:

```bash
blisp write --chip bl60x --reset -p /dev/ttyUSB0 name_of_firmware.bin
```

If you wish to see additional debugging, set the environmental
variable LIBSERIALPORT_DEBUG before running. You can either export this
in your shell or change it for a single run via

```bash
  LIBSERIALPORT_DEBUG=y ./a.out write -c bl70x -p /dev/tty.usbmodem0000000200001
```

Because this is done at the lowest level of serial communication, the
displays aren't packet-aware or know about the chip's command set or such.
This is really only useful for debugging systems-level issues withing
the device or blisp itself.

## Troubleshooting

### macOS

Depending on your current system security settings, modern versions of macOS requires all software to be notarised before you are able to execute it. This is specially true for software that is downloaded directly from the internet.

If that is the case, you will get an error that looks like the following:
> **“blisp” cannot be opened because the developer cannot be verified.**
>
> macOS cannot verify that this app is free from malware.

In that case, you will need to remove the *quarantine* flag that macOS adds to the executable. After that you should be able to run **blisp** as normal.

```bash
xattr -d com.apple.quarantine blisp
```
