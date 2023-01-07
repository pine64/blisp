// SPDX-License-Identifier: MIT
#ifndef BLISP_BLISP_EASY_H
#define BLISP_BLISP_EASY_H

#include <stdio.h>
#include "blisp.h"

struct blisp_easy_transport {
  uint8_t type;  // 0 - memory, 1 - FILE file_handle
  union {
    FILE* file_handle;
    struct {
      void* data_location;
      uint32_t data_size;
      uint32_t current_position;
    } memory;
  } data;
};

enum blisp_easy_error {
  BLISP_EASY_ERR_TRANSPORT_ERROR = -100,
  BLISP_EASY_ERR_CHECK_IMAGE_FAILED = -101
};

typedef void (*blisp_easy_progress_callback)(uint32_t current_value,
                                             uint32_t max_value);

struct blisp_easy_transport blisp_easy_transport_new_from_file(FILE* file);

int32_t blisp_easy_load_segment_data(
    struct blisp_device* device,
    uint32_t segment_size,
    struct blisp_easy_transport* segment_transport,
    blisp_easy_progress_callback progress_callback);

int32_t blisp_easy_load_ram_image(
    struct blisp_device* device,
    struct blisp_easy_transport* image_transport,
    blisp_easy_progress_callback progress_callback);

int32_t blisp_easy_flash_write(struct blisp_device* device,
                               struct blisp_easy_transport* data_transport,
                               uint32_t flash_location,
                               uint32_t data_size,
                               blisp_easy_progress_callback progress_callback);

#endif  // BLISP_BLISP_EASY_H
