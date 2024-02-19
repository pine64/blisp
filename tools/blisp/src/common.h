// SPDX-License-Identifier: MIT
#ifndef BLISP_COMMON_H
#define BLISP_COMMON_H

#include <stdint.h>
#include <blisp.h>
#include <argtable3.h>

// https://gcc.gnu.org/onlinedocs/cpp/Stringizing.html
#define DEFAULT_BAUDRATE 460800
#define STR(x) #x
#define XSTR(x) STR(x)

int32_t blisp_common_prepare_flash(struct blisp_device* device);
void blisp_common_progress_callback(uint32_t current_value, uint32_t max_value);
int32_t blisp_common_init_device(struct blisp_device* device, struct arg_str* port_name, struct arg_str* chip_type, uint32_t baudrate);

#endif  // BLISP_COMMON_H
