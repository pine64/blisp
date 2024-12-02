// SPDX-License-Identifier: MIT
#include <stdlib.h>
#include <string.h>
#include "../../data/bl60x_eflash_loader.h"
#include "blisp.h"

int64_t blisp_chip_bl60x_get_eflash_loader([[maybe_unused]] uint8_t clk_type, uint8_t** firmware_buf_ptr)
{
  uint8_t* firmware_buf = malloc(sizeof(bl60x_eflash_loader_bin));
  memcpy(firmware_buf, bl60x_eflash_loader_bin, sizeof(bl60x_eflash_loader_bin));
  *(firmware_buf + 0xE0) = 4; // TODO: 40 MHz clock
  *firmware_buf_ptr = firmware_buf;
  return sizeof(bl60x_eflash_loader_bin);
}

struct blisp_chip blisp_chip_bl60x = {
    .type = BLISP_CHIP_BL60X,
    .type_str = "bl60x",
    .usb_isp_available = false,
    .default_xtal = "40m",
    .handshake_byte_multiplier = 0.006f,
    .load_eflash_loader = blisp_chip_bl60x_get_eflash_loader,
    .tcm_address = 0x22010000
};
