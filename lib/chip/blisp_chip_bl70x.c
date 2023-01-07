// SPDX-License-Identifier: MIT
#include "blisp.h"

struct blisp_chip blisp_chip_bl70x = {
    .type = BLISP_CHIP_BL70X,
    .type_str = "bl70x",
    .usb_isp_available = true,
    .default_xtal = "32m",
    .handshake_byte_multiplier = 0.003f,
    .needs_eflash_loader = true
};
