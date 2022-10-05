#ifndef _BLISP_CHIP_H
#define _BLISP_CHIP_H

#include <stdint.h>
#include <stdbool.h>

enum blisp_chip_type {
    BLISP_CHIP_BL70X
};

struct blisp_chip { // TODO: Move elsewhere?
    enum blisp_chip_type type;
    const char* type_str;
    bool usb_isp_available;
};

extern struct blisp_chip blisp_chip_bl70x;

#endif