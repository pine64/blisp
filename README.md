# Bouffalo Labs ISP tool & library

Tool and library for flashing their RISC-V MCUs.

# Supported MCUs

- [ ] BL602 / BL604
- [X] BL702 / BL704 / BL706
- [ ] BL606P
- [ ] BL616 / BL618
- [ ] BL808

# To Do

- [ ] Another code style
- [ ] Finalize API
- [ ] SDIO and JTAG support

# Build

This project is not self-contained. After you check out the master project
from GitHub, you must fetch the additional vendor libraries via:
$ git submodule update --init --recursive

If vendor/argtable3 and vendor/libserialport/ are empty, this step has
failed and should be investigated.

# Usage

  blisp write -c <chip_type> [-p <port_name>] <input>
  blisp [--help] [--version]

A typical invocation may therefore be:

  blisp write -c bl70x -p /dev/tty.usbmodem0000000200001

If you wish to see a painful level of debugging, set the environmental
variable LIBSERIALPORT_DEBUG before running. You can either export this
in your shell or change it for a single run via

  LIBSERIALPORT_DEBUG=y ./a.out write -c bl70x -p /dev/tty.usbmodem0000000200001

Because this is done at the lowest level of serial communication, the
displays aren't packet-aware or know about the chip's command set or such.
This is really only useful for debugging systems-level issues withing
the device or blisp itself.


