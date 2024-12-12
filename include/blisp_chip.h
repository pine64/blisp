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
  int64_t (*load_eflash_loader)(uint8_t clk_type, uint8_t** firmware_buf_ptr);
  uint32_t tcm_address;
};

extern struct blisp_chip blisp_chip_bl60x;
extern struct blisp_chip blisp_chip_bl70x;
extern struct blisp_chip blisp_chip_bl808;
extern struct blisp_chip blisp_chip_bl61x;

extern struct bl808_bootheader_t bl808_header;
void fill_crcs(struct bl808_bootheader_t *bh);

#endif
