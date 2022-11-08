#include "blisp.h"

struct blisp_chip blisp_chip_bl60x = {
    .type = BLISP_CHIP_BL60X,
    .type_str = "bl60x",
    .usb_isp_available = false,
    .default_eflash_loader_xtal = "40m",
    .handshake_byte_multiplier = 0.006f,
};
