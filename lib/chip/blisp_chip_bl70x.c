// SPDX-License-Identifier: MIT
#include <stdlib.h>
#include <string.h>
#include "../../data/bl70x_eflash_loader.h"
#include "blisp.h"

int64_t blisp_chip_bl70x_get_eflash_loader([[maybe_unused]] uint8_t clk_type, uint8_t** firmware_buf_ptr)
{
  uint8_t* firmware_buf = malloc(sizeof(bl70x_eflash_loader_bin));
  memcpy(firmware_buf, bl70x_eflash_loader_bin, sizeof(bl70x_eflash_loader_bin));
  *(firmware_buf + 0xE0) = 1; // TODO: 32 MHz clock
  *firmware_buf_ptr = firmware_buf;
  return sizeof(bl70x_eflash_loader_bin);
}

struct blisp_chip blisp_chip_bl70x = {
    .type = BLISP_CHIP_BL70X,
    .type_str = "bl70x",
    .usb_isp_available = true,
    .default_xtal = "32m",
    .handshake_byte_multiplier = 0.003f,
    .load_eflash_loader = blisp_chip_bl70x_get_eflash_loader,
    .tcm_address = 0x22010000
};

