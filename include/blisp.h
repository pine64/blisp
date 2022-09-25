#ifndef _LIBBLISP_H
#define _LIBBLISP_H

#include <stdint.h>

#include "blisp_chip.h"

struct blisp_device {
    struct blisp_chip* chip;
    void* serial_port;
    bool is_usb;
    uint32_t current_baud_rate;
    uint8_t rx_buffer[5000]; // TODO:
    uint8_t tx_buffer[5000];
    uint16_t error_code;
};

struct blisp_boot_info {
    uint8_t boot_rom_version[4];
    uint8_t chip_id[8]; // TODO: BL60X only 6 bytes
};

int32_t blisp_device_init(struct blisp_device* device, struct blisp_chip* chip);
int32_t blisp_device_open(struct blisp_device* device, const char* port_name);
int32_t blisp_device_handshake(struct blisp_device* device);
int32_t blisp_device_get_boot_info(struct blisp_device* device, struct blisp_boot_info* boot_info);
void blisp_device_close(struct blisp_device* device);

#endif