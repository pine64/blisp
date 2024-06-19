// SPDX-License-Identifier: MIT
#ifndef BLISP_COMMON_H
#define BLISP_COMMON_H

#include <stdint.h>
#include <blisp.h>
#include <argtable3.h>

blisp_return_t blisp_common_prepare_flash(struct blisp_device* device,
                                          bool goto_eflash_loader);
void blisp_common_progress_callback(uint32_t current_value, uint32_t max_value);
int32_t blisp_common_init_device(struct blisp_device* device, struct arg_str* port_name, struct arg_str* chip_type);

#endif  // BLISP_COMMON_H
