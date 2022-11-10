#ifndef _BLISP_CHIP_H
#define _BLISP_CHIP_H

#include <stdint.h>
#include <stdbool.h>

enum blisp_chip_type {
    BLISP_CHIP_BL60X,
    BLISP_CHIP_BL70X,
    BLISP_CHIP_BL808
};

struct blisp_chip { // TODO: Move elsewhere?
    enum blisp_chip_type type;
    const char* type_str;
    bool usb_isp_available;
    float handshake_byte_multiplier;
    const char* default_eflash_loader_xtal; // TODO: Make this selectable
};

extern struct blisp_chip blisp_chip_bl60x;
extern struct blisp_chip blisp_chip_bl70x;
extern struct blisp_chip blisp_chip_bl808;

#endif