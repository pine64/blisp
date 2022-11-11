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

# Updating Pinecil V2: How to build BLISP Flasher

_Note: This has been tested on x86-64. The build process also works on aarch64 and armv7._
## Linux Steps

⛔ Do not use the Pinecil DC barrel jack while updating firmware or you may destroy your PC. ⛔

1. **Get Source Code**: this code only works for Pinecil V2 and not the older V1 model which has different hardware.

   Click the [Green Code button here](https://github.com/pine64/blisp), then download the zip and extract it. 

2. **Linux set-up**
```
git clone --recursive https://github.com/pine64/blisp.git
cd blisp
mkdir build && cd build
cmake -DBLISP_BUILD_CLI=ON ..
cmake --build .
```
  Note: it will be in tools/blisp folder and can later be run as with flags as ` ./tools/blisp/blisp`

3. ` mkdir tools/blisp/data`

4. Get eflash_loader_32m.bin
   
   a. Download [eflash*.bin here](https://github.com/blisp/tree/master/eflash).
   
   b. Unzip and move it to the bl70x folder  `unzip eflash_loader_32m.zip -d tools/blisp/data/bl70x/`

5. **Get V2 firmware** from Github IronOS

   a. Download the newest stable [firmware release here](https://github.com/Ralim/IronOS).
   
   b. Or download the Beta firmware, go to IronOS Actions like [here](https://github.com/Ralim/IronOS/actions/runs/3409043548) or select a newer dated Action.
      Scroll to the very bottom of the actions page and download **Pinecilv2**. This link is to [beta 2.20](https://github.com/Ralim/IronOS/actions/runs/3409043548)
   
   d. Extract **Pinecilv2.zip** and select a single language file (English = Pinecilv2_EN.bin).

6.  Put Pinecilv2_EN.bin (or selected language) into `tools/blisp/data/bl70x/`
can delete all the rest of the Pinecilv2**.zip as it is not needed.

7. Putting Pinecil V2 into Flasing Mode:

      a. Hold the minus `(-)` button down first and Keep holding `(-)`.
      
      b. Then Plug in the USB-C cable. Wait another 15-20sec before releasing the minus `(-)` button.
      
      c. If successful, the screen will be black/blank which means you are are in flasher mode and ready to upload firmware.
      
      d. If this fails, see [troubleshooting below](https://github.com/blisp/blob/master/README.md#troubleshooting).

8. Flash the firmware onto Pinecil V2. If a different language was selected, replace `Pinecilv2_**.bin` below with the chosen file.
` sudo ./tools/blisp/blisp write -c bl70x --reset Pinecilv2_EN.bin`


9. Done, Congradulations! Unplug from the PC and estart V2. Hold down the minus `(-)` button to see the new firmware version.

10. Before making menu changes, is recommended to first [Restore Settings to Default](https://github.com/Ralim/IronOS/blob/dev/Documentation/GettingStarted.md#settings-menu). Simply go to Advanced settings > Restore default settings, confirm using the `(+)` button. This sets all menu items to defaults, keeps the same firmware version, and does not affect any Boot-up logo art if applicable.

## Troubleshooting
1. If the Pinecil V2 or device fails to connect to the PC:

    a. try different cable: usb-C to C is recommended over Usb-A, especially if you are having issues.
    
    b. try different Usb ports (usb-c recommended)
    
    c. try a different PC/laptop
    
2. It is important to hold down the `(-)` minus button _before_ plugging in the Usb-c cable, and do not release the button for another 15-20 seconds. Try to hold it a little longer before releasing if your computer is slow and it is not working. In rare circumstances on fussy USB ports, keep holding the `(-)` for the entire update.

3. If all of this fails, then join one of the [Live Community Chat channels linked](https://wiki.pine64.org/wiki/Pinecil#Community_links) in the Pinecil Wiki as volunteers there might be able to help.

4. Open a an new issue ticket in this Github/Blisp https://github.com/pine64/blisp/issues


# To Do

- [ ] Another code style
- [ ] Finalize API
- [ ] SDIO and JTAG support
