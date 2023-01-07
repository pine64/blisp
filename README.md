# Bouffalo Labs ISP tool & library

Open source tool and library for flashing Bouffalo RISC-V MCUs.

**NOTE:** Library API and `blisp` tool cli arguments are not stable yet.

# Supported MCUs

- [x] `bl60x` - BL602 / BL604
- [x] `bl70x` - BL702 / BL704 / BL706
- [ ] `bl606p` - BL606P
- [ ] `bl61x` - BL616 / BL618
- [ ] `bl808` - BL808

# Supported OS
- [x] Windows
- [x] Linux
- [ ] Apple (WIP: work-in-progress)

# Building

## Clone repository

If you have not cloned this repository locally; clone the git repository locally by running

```bash
git clone --recursive https://github.com/pine64/blisp.git
cd blisp
```

## Build the library and command line utility

For building `blisp` command line tool, use following commands:

```bash
mkdir build && cd build
cmake -DBLISP_BUILD_CLI=ON ..
cmake --build .
```

# Usage

For BL70X, BL61X, BL808 and BL606P, connected via USB, you can use following command, which will auto-detect serial port:

```bash
blisp write --chip bl70x --reset name_of_firmware.bin
```

For BL60X, you need to specify also the serial port path:

```bash
blisp --chip bl60x --reset -p /dev/ttyUSB0 name_of_firmware.bin
```

# How to flash Pinecil V2

Check out the [wiki page](https://github.com/pine64/blisp/wiki/Update-Pinecil-V2).
