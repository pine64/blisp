# Bouffalo Labs ISP tool & library

Open source tool and library for flashing Bouffalo RISC-V MCUs.

**NOTE:** Library API and `blisp` tool cli arguments are not stable yet.

# Supported MCUs

- [X] `bl60x` - BL602 / BL604
- [X] `bl70x` - BL702 / BL704 / BL706 
- [ ] `bl606p` - BL606P
- [ ] `bl61x` - BL616 / BL618
- [ ] `bl808` - BL808

# Supported Devices
- [X] [Pinecil V2](https://wiki.pine64.org/wiki/Pinecil)

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

# Updating Pinecil V2: How to build BLISP Flasher for Linux

_Note: This has been tested on x86-64. The build process also works on aarch64 and armv7._
## Steps

⛔ Do not use the Pinecil DC barrel jack while updating firmware or you may destroy your PC. ⛔

1. **Get Source Code** here for Linux BLISP flasher. This code only works for Pinecil V2 and not the older V1 model which has different hardware.
**https://github.com/pine64/blisp**
Click green Code button, then Download the Zip and extract it.

2. **Linux set-up**
```
git clone --recursive https://github.com/pine64/blisp.git
cd blisp
mkdir build && cd build
cmake -DBLISP_BUILD_CLI=ON ..
cmake --build .
```
   It will be in tools/blisp folder, so can later be run as  ` ./tools/blisp/blisp`

3. ` mkdir tools/blisp/data`

4. Get eflash_loader_32m.bin, then `unzip eflash_loader_32m.zip -d tools/blisp/data/bl70x/`
Download eflash*.bin here.
Unzip it and put it into the bl70x folder.

5. **Get V2 beta firmware** from Github IronOS Actions like this [link below](https://github.com/Ralim/IronOS/actions/runs/3409043548) or a newer dated Action.
Scroll to the very bottom of this linked page: https://github.com/Ralim/IronOS/actions/runs/3409043548
Get **Pinecilv2.zip** and extract it (English =  Pinecilv2_EN.bin file).

6.  Put the  Pinecilv2_EN.bin (or other selected language) into  `tools/blisp/data/bl70x/`
can delete all the rest of the Pinecilv2**.zip as it is not needed.

7. Putting Pinecil V2 into Flasing Mode:
      a. Hold the minus (-) button down first and Keep holding (-).
      b. Then Plug in the USB-C cable. Wait another 15-20sec before releasing the minus `(-)` button.
      c. If successful, the screen will be black/blank which means you are are in flasher mode and ready to upload firmware.
      d. If this fails, see [troubleshooting below](https://github.com/blisp/blob/master/README.md#troubleshooting).

8. Execute ` sudo ./tools/blisp/blisp write -c bl70x --reset Pinecilv2_EN.bin`
or whichever language Pinecilv2**.bin you extracted earlier.

9. Done. Unplug from the PC and restart V2. Hold down the minus `(-)` button to see the new firmware version.


# Troubleshooting
1. If the Pinecil V2 or device fails to connect to the PC:

    (a) try different cables (usb-C to C is recommended over Usb-A)
    (b) try different Usb port
    (c) try a different PC/laptop
    
2. It is important to hold down the (-) minus button _before_ plugging in the Usb-c cable, and do not release the button for another 15-20 seconds. Try to hold it a longer before releasing if your computer is slow and it is not working. In rare circumstances on fussy USB ports, keep holding the (-) for the entire update.
3. If all of this fails, then join one of the Community Live Chat channels linked in the Pinecil Wiki where volunteers might be able to help: https://wiki.pine64.org/wiki/Pinecil#Community_links
4. Open a an new issue ticket in this Github/Blisp https://github.com/pine64/blisp/issues


# To Do

- [ ] Another code style
- [ ] Finalize API
- [ ] SDIO and JTAG support
