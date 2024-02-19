// SPDX-License-Identifier: MIT
#ifndef _LIBBLISP_H
#define _LIBBLISP_H

#include <stdint.h>
#include "blisp_chip.h"
#include "error_codes.h"

struct blisp_segment_header {
  uint32_t dest_addr;
  uint32_t length;
  uint32_t reserved;
  uint32_t crc32;
};

struct blisp_device {
  struct blisp_chip* chip;
  void* serial_port;
  bool is_usb;
  uint32_t current_baud_rate;
  uint8_t rx_buffer[5000];  // TODO:
  uint8_t tx_buffer[5000];
  uint16_t error_code;
};

struct blisp_boot_info {
  uint8_t boot_rom_version[4];
  uint8_t chip_id[8];  // TODO: BL60X only 6 bytes
};

// TODO: Refactor variable names, so all will follow same semantic, like
// image_run, image_check etc.

int32_t blisp_device_init(struct blisp_device* device, struct blisp_chip* chip);
int32_t blisp_device_open(struct blisp_device* device, const char* port_name, uint32_t baudrate);
int32_t blisp_device_handshake(struct blisp_device* device, bool in_ef_loader);
int32_t blisp_device_get_boot_info(struct blisp_device* device,
                                   struct blisp_boot_info* boot_info);
int32_t blisp_device_load_boot_header(struct blisp_device* device,
                                      uint8_t* boot_header);
int32_t blisp_device_load_segment_header(
    struct blisp_device* device,
    struct blisp_segment_header* segment_header);
int32_t blisp_device_load_segment_data(struct blisp_device* device,
                                       uint8_t* segment_data,
                                       uint32_t segment_data_length);
int32_t blisp_device_write_memory(struct blisp_device* device,
                                  uint32_t address,
                                  uint32_t value,
                                  bool wait_for_res);
int32_t blisp_device_check_image(struct blisp_device* device);
int32_t blisp_device_run_image(struct blisp_device* device);
int32_t blisp_device_flash_erase(struct blisp_device* device,
                                 uint32_t start_address,
                                 uint32_t end_address);
int32_t blisp_device_flash_write(struct blisp_device* device,
                                 uint32_t start_address,
                                 uint8_t* payload,
                                 uint32_t payload_size);
int32_t blisp_device_program_check(struct blisp_device* device);
int32_t blisp_device_reset(struct blisp_device* device);
void blisp_device_close(struct blisp_device* device);

blisp_return_t bl808_load_clock_para(struct blisp_device* device,
                                     bool irq_en, uint32_t baudrate);
blisp_return_t bl808_load_flash_para(struct blisp_device* device);

#endif
