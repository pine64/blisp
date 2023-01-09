// SPDX-License-Identifier: MIT
#include "blisp.h"

struct blisp_chip blisp_chip_bl808 = {
    .type = BLISP_CHIP_BL808,
    .type_str = "bl808",
    .usb_isp_available = true, // TODO: Only for BL808D :-(
    .default_xtal = "-", // ?
    .handshake_byte_multiplier = 0.003f,
    .get_eflash_loader = NULL
};
