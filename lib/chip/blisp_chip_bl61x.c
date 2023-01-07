// SPDX-License-Identifier: MIT
#include "blisp.h"

struct blisp_chip blisp_chip_bl61x = {
    .type = BLISP_CHIP_BL61X,
    .type_str = "bl808",
    .usb_isp_available = true,
    .default_xtal = "-", // ?
    .handshake_byte_multiplier = 0.003f,
    .needs_eflash_loader = false
};
