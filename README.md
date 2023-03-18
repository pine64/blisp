[![Hits](https://hits.seeyoufarm.com/api/count/incr/badge.svg?url=https%3A%2F%2Fgithub.com%2Fpine64%2Fblisp&count_bg=%235791AC&title_bg=%23555555&icon=airplayaudio.svg&icon_color=%23D2D9DD&title=hits&edge_flat=false)](https://github.com/pine64/blisp/wiki/Update-Pinecil-V2)
[![GitHub all downloads](https://img.shields.io/github/downloads/pine64/blisp/total?color=5791ac&logo=docusign&logoColor=white)](https://github.com/pine64/blisp/releases/tag/v0.0.3)
[![Discord](https://img.shields.io/discord/463237927984693259?color=5791ac&logo=discord&logoColor=white)](https://discord.com/invite/pine64)
[![GitHub release](https://img.shields.io/github/v/release/pine64/blisp?color=5791ac)](https://github.com/pine64/blisp/releases/tag/v0.0.3)

<img src="./img/Gradient-white-blue-03.png" align="left" width="60" > <br clear="right" />
# BLISP 

Bouffalo Labs ISP tool & library: an open source tool flashing Bouffalo RISC-V MCUs.

**NOTE:** Library API and `blisp` tool cli arguments are not stable yet.

## Supported MCUs

- [x] `bl60x` - BL602 / BL604 / TG7100C / LF686 / LF688
- [x] `bl70x` - BL702 / BL704 / BL706
- [ ] `bl70xl` - BL702L / BL704L
- [ ] `bl606p` - BL606P
- [ ] `bl61x` - BL616 / BL618
- [ ] `bl808` - BL808

## Supported Devices 
 | System  | <img width="15" src="https://cdn.simpleicons.org/Windows11/000000" /> Windows | <img width="15" src="https://cdn.simpleicons.org/Apple" /> MacOS| <img width="17" src="https://cdn.simpleicons.org/Linux/000000" /> Linux| <img width="15" src="https://cdn.simpleicons.org/Freebsd/000000" /> FreeBSD |
 | :-----: | :-----: | :--: | :--: | :-----: |
 | Pinecil V2  |:heavy_check_mark:|:heavy_check_mark:| :heavy_check_mark:|:heavy_check_mark:   |
 | Pinecone  |:heavy_check_mark:|:heavy_check_mark:|:heavy_check_mark:|:heavy_check_mark:   |
 <br>
 
## How to update Pinecil V2

Check out the [wiki page](https://github.com/pine64/blisp/wiki/Update-Pinecil-V2).
<br><br>

## Building from code

### Clone repository

If you have not cloned this repository locally; clone the git repository locally by running

```bash
git clone --recursive https://github.com/pine64/blisp.git
cd blisp
git submodule update --init --recursive
```

### Build the library and command line utility

For building `blisp` command line tool, use following commands:

```bash
mkdir build && cd build
cmake -DBLISP_BUILD_CLI=ON ..
cmake --build .
```
#### Need more build details? [See here](https://github.com/pine64/blisp/wiki/Update-Pinecil-V2#build-blisp-flasher-from-code).

## Usage

For BL70X, BL61X, BL808 and BL606P, connected via USB, you can use following command, which will auto-detect serial port:

```bash
.\blisp.exe write --chip=bl70x --reset .\name_of_firmware.bin
or
.\blisp.exe write -c bl70x --reset .\name_of_firmware.bin
```

For BL60X, you need to specify also the serial port path:

```bash
blisp write --chip bl60x --reset -p /dev/ttyUSB0 name_of_firmware.bin
```

