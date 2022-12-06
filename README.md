# Bouffalo Labs ISP tool & library

Open source tool and library for flashing Bouffalo RISC-V MCUs.

**NOTE:** Library API and `blisp` tool cli arguments are not stable yet.

# Supported MCUs

- [x] `bl60x` - BL602 / BL604
- [x] `bl70x` - BL702 / BL704 / BL706
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
blisp write --chip bl70x --reset name_of_firmware.bin
```

For BL60X, you need to specify also the serial port path:

```bash
blisp --chip bl60x --reset -p /dev/ttyUSB0 name_of_firmware.bin
```

# Updating Pinecil V2: How to build BLISP Flasher

_Note: This has been tested on x86-64. The build process also works on aarch64 and armv7._
## Linux Steps

⛔ Do not use the Pinecil DC barrel jack while updating firmware or you may destroy your PC. ⛔

1. **Linux set-up**
```
git clone --recursive https://github.com/pine64/blisp.git
cd blisp
mkdir build && cd build
cmake -DBLISP_BUILD_CLI=ON ..
cmake --build .
mkdir tools/blisp/data
mkdir -p tools/blisp/data/bl70x
```
  Note: the blisp command will now be in build/tools/blisp/ folder and could later be run with flags as ` ./tools/blisp/blisp` unless you cd into that folder.

2. Get and cp or mv eflash_loader_32m.bin  to  bl70x folder
```
/build/tools/blisp/data/bl70x/eflash_loader_32m.bin
``` 
   a. Download [eflash*32m.bin here](https://github.com/River-b/blisp/tree/master/eflash).
   
   b. Move eflash*32m.bin to the folder  build/tools/data/bl70x 
   c. If it is a Zip, then unzip & move it.
```
unzip eflash_loader_32m.zip -d tools/blisp/data/bl70x/
```

3. **Get V2 firmware** from Github Ralim's IronOS

   a. Download the newest stable [firmware release here](https://github.com/Ralim/IronOS).

   b. Or download the Beta firmware [IronOS here](https://github.com/Ralim/IronOS/actions/runs/3545583488)
      Scroll to the very bottom of the page and download **Pinecilv2**. This link is to [beta that has BLE and also works for EPR chargers](https://github.com/Ralim/IronOS/actions/runs/3545583488)
   
   d. Extract **Pinecilv2.zip** and select a single language file (English = ```Pinecilv2_EN.bin```).
   
   e. Move the Pinecilv2_EN.bin (or selected language) into the same folder as the blisp command.
```
build/tools/blisp/Pinecilv2_EN.bin
```
can delete all the rest of the Pinecilv2**.zip as it is not needed.

4. Connect Pinecil to PC/laptop: long hold [`-`]  , then connect cable. Can release the [-] after about 15-20second. V2 screen should be Empty/black,  if not, then repeat connection, or find another cable/port. Pinecil connects as a serial COM port.

5. If this fails, see [troubleshooting below](https://github.com/River-b/blisp/blob/master/README.md#troubleshooting).

6. If you are in  the folder  `blisp/build/tools/blisp/`   then execute 
```
sudo ./blisp write -c bl70x --reset Pinecilv2_EN.bin
```
Note: if a different language was selected, replace `Pinecilv2_**.bin` above with the chosen file name.

7. Almost done: unplug from the PC and restart V2. Hold down the minus `(-)` button to see the new version number.

8. Before making menu changes, it is recommended to first [Restore Settings to Default](https://github.com/Ralim/IronOS/blob/dev/Documentation/GettingStarted.md#settings-menu).
   Simply go to Advanced settings > Restore default settings, confirm using the `(+)` button. This sets all menu items to defaults, keeps the same firmware version, and does not affect any Boot-up logo art if applicable. Setting  defaults first avoids unexpected behavior due to some changes in upgrades.
   
9. Congradulations, and [Stay Fluxey, my friends!](https://www.reddit.com/r/PINE64official/comments/xk9vxu/most_interesting_man_in_the_world_i_dont_always/?utm_source=share&utm_medium=web2x&context=3)

## Troubleshooting
1. If the Pinecil V2 fails to connect to the PC, check the `dmesg` command output.

    a. try different cable: usb-C to C is recommended over Usb-A, especially if you are having issues.
    
    b. don't use a USB hub, directly connect to the USB port on the PC or laptop.
    
    c. try different Usb ports (usb-c recommended). Sometimes the rear ports on a PC are better because they are directly connected to the motherboard.
    
    d. try a different PC/laptop
    
2. It is important to hold down the `(-)` minus button _before_ plugging in the Usb-c cable, and do not release the button for another 15-20 seconds. Try to hold it a little longer before releasing if your computer is slow and it is not working. In rare circumstances on fussy USB ports, keep holding the `(-)` for the entire update.

3. If all of this fails, then join one of the [Live Community Chat channels linked](https://wiki.pine64.org/wiki/Pinecil#Community_links) in the Pinecil Wiki as volunteers there might be able to help.

4. Open a an new issue ticket in this Github/Blisp flasher at https://github.com/pine64/blisp/issues

5. See [Pinecil Wiki](https://wiki.pine64.org/wiki/Pinecil) for hardware information.

6. See [Github Ralim's IronOS](https://ralim.github.io/IronOS/#getting-started) for firmware/software information. This is only the Flasher that loads the firmware; all Pinecil firmware documents and menu instructions are in IronOS.


# To Do

- [ ] Another code style
- [ ] Finalize API
- [ ] SDIO and JTAG support
