// SPDX-License-Identifier: MIT
#include "blisp.h"
#include <stddef.h>

struct blisp_chip blisp_chip_bl808 = {
    .type = BLISP_CHIP_BL808,
    .type_str = "bl808",
    .usb_isp_available = true, // TODO: Only for BL808D :-(
    .default_xtal = "-", // XXX: bfl software marks this as "Auto (0x07)"
    .handshake_byte_multiplier = 0.006f,
    .load_eflash_loader = NULL
};
