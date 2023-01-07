// SPDX-License-Identifier: MIT
#ifndef _BLISP_CHIP_H
#define _BLISP_CHIP_H

#include <stdbool.h>
#include <stdint.h>

enum blisp_chip_type {
  BLISP_CHIP_BL60X,
  BLISP_CHIP_BL70X,
  BLISP_CHIP_BL606P,
  BLISP_CHIP_BL808,
  BLISP_CHIP_BL61X,
};

struct blisp_chip {  // TODO: Move elsewhere?
  enum blisp_chip_type type;
  const char* type_str;
  bool usb_isp_available;
  float handshake_byte_multiplier;
  const char* default_xtal;  // TODO: Make this selectable
  bool needs_eflash_loader;
};

extern struct blisp_chip blisp_chip_bl60x;
extern struct blisp_chip blisp_chip_bl70x;
extern struct blisp_chip blisp_chip_bl808;
extern struct blisp_chip blisp_chip_bl61x;

#endif