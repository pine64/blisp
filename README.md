# Bouffalo Labs ISP tool & library

Tool and library for flashing their RISC-V MCUs.

**NOTE:** Library API and `blisp` tool cli arguments are not stable yet.

# Supported MCUs

- [X] `bl60x` - BL602 / BL604
- [X] `bl70x` - BL702 / BL704 / BL706 
- [ ] `bl606p` - BL606P
- [ ] `bl61x` - BL616 / BL618
- [ ] `bl808` - BL808

# Building
For building `blisp` command line tool, use following commands:

```bash
git clone --recursive https://github.com/pine64/blisp.git
cd blisp
mkdir build && cd build
cmake -DBLISP_BUILD_CLI=ON ..
cmake --build .
```

# Usage

For BL70X, BL61X, BL808 and BL606P, connected via USB, you can use following command, which will auto-detect serial port:

```bash
blisp --chip bl70x --reset name_of_firmware.bin
```

For BL60X, you need to specify also the serial port path:

```bash
blisp --chip bl60x --reset -p /dev/ttyUSB0 name_of_firmware.bin
```

# Troubleshooting
1. If the Pinecil V2 or device fails to connect to the PC:

    (a) try different cables (usb-C to C is recommended over Usb-A)
    
    (b) try different Usb ports
    
    (c) try a different PC/laptop
    
2. It is important to hold down the (-) minus button _before_ plugging in the Usb-c cable, and do not release the button for another 15-20 seconds. Try to hold it a longer before releasing if your computer is slow and it is not working. In rare circumstances on fussy USB ports, keep holding the (-) for the entire update.
3. If all of this fails, then join one of the Community Live Chat channels linked in the Pinecil Wiki where volunteers might be able to help: https://wiki.pine64.org/wiki/Pinecil#Community_links
4. And/or open a an new issue ticket in this Github/Blisp https://github.com/pine64/blisp/issues


# To Do

- [ ] Another code style
- [ ] Finalize API
- [ ] SDIO and JTAG support
