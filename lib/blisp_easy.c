// SPDX-License-Identifier: MIT
#include "blisp_easy.h"
#include "blisp_struct.h"

#include <inttypes.h>

static int64_t blisp_easy_transport_read(struct blisp_easy_transport* transport,
                                         void* buffer,
                                         uint32_t size) {
  if (transport->type == 0) {
  } else {
    return fread(buffer, size, 1, transport->data.file_handle);
  }
}

static void blisp_easy_report_progress(blisp_easy_progress_callback callback,
                                       uint32_t current_value,
                                       uint32_t max_value) {
  if (callback != NULL) {
    callback(current_value, max_value);
  }
}

struct blisp_easy_transport blisp_easy_transport_new_from_file(FILE* file) {
  struct blisp_easy_transport transport = {.type = 1, .data.file_handle = file};
  return transport;
}

int32_t blisp_easy_load_segment_data(
    struct blisp_device* device,
    uint32_t segment_size,
    struct blisp_easy_transport* segment_transport,
    blisp_easy_progress_callback progress_callback) {
  int32_t ret;

  uint32_t sent_data = 0;
  uint32_t buffer_size = 0;
  uint8_t buffer[4092];

  blisp_easy_report_progress(progress_callback, 0, segment_size);

  while (sent_data < segment_size) {
    buffer_size = segment_size - sent_data;
    if (buffer_size > 4092) {
      buffer_size = 4092;
    }
    blisp_easy_transport_read(segment_transport, buffer, buffer_size); // TODO: Error Handling
    ret = blisp_device_load_segment_data(device, buffer,
                                         buffer_size);
    if (ret < BLISP_OK) {
      // TODO: Error logging fprintf(stderr, "Failed to load segment data. (ret
      // %d)\n", ret);
      return BLISP_EASY_ERR_TRANSPORT_ERROR;
    }
    sent_data += buffer_size;
    blisp_easy_report_progress(progress_callback, sent_data, segment_size);
  }
  return 0;
}

int32_t blisp_easy_load_ram_image(
    struct blisp_device* device,
    struct blisp_easy_transport* image_transport,
    blisp_easy_progress_callback progress_callback) {
  int32_t ret;

  struct bfl_boot_header image_boot_header;
  // TODO: Error handling
  blisp_easy_transport_read(image_transport, &image_boot_header, 176);

  ret = blisp_device_load_boot_header(device, (uint8_t*)&image_boot_header);
  if (ret != BLISP_OK) {
    // TODO: Error printing: fprintf(stderr, "Failed to load boot header.\n");
    return BLISP_EASY_ERR_TRANSPORT_ERROR;
  }

  {
    for (uint8_t seg_index = 0;
         seg_index < image_boot_header.segment_info.segment_cnt; seg_index++) {
      struct blisp_segment_header segment_header = {0};
      blisp_easy_transport_read(image_transport, &segment_header,
                                16);  // TODO: Error handling

      ret = blisp_device_load_segment_header(device, &segment_header);
      if (ret != 0) {
        // TODO: Error printing: fprintf(stderr, "Failed to load segment
        // header.");
        return BLISP_EASY_ERR_TRANSPORT_ERROR;
      }
      // TODO: Info printing: printf("Flashing %d. segment\n", seg_index + 1);

      ret = blisp_easy_load_segment_data(device, segment_header.length,
                                         image_transport, progress_callback);
      if (ret != 0) {
        return BLISP_EASY_ERR_TRANSPORT_ERROR;
      }
    }
  }

  ret = blisp_device_check_image(device);
  if (ret != BLISP_OK) {
    // TODO: Error printing: fprintf(stderr, "Failed to check image.\n");
    return BLISP_EASY_ERR_CHECK_IMAGE_FAILED;
  }

  return BLISP_OK;
}

int32_t blisp_easy_flash_write(struct blisp_device* device,
                               struct blisp_easy_transport* data_transport,
                               uint32_t flash_location,
                               uint32_t data_size,
                               blisp_easy_progress_callback progress_callback) {
  int32_t ret;
  uint32_t sent_data = 0;
  uint32_t buffer_size = 0;
  uint8_t buffer[8184];
  blisp_easy_report_progress(progress_callback, 0, data_size);

  while (sent_data < data_size) {
    buffer_size = data_size - sent_data;
    if (buffer_size > 2052) {
      buffer_size = 2052;
    }
    blisp_easy_transport_read(data_transport, buffer, buffer_size); // TODO: Error Handling
    ret = blisp_device_flash_write(device, flash_location + sent_data, buffer,
                                   buffer_size);
    if (ret < BLISP_OK) {
      // TODO: Error logigng:  fprintf(stderr, "Failed to write firmware! (ret:
      // %d)\n", ret);
      return ret;
    }
    sent_data += buffer_size;
    blisp_easy_report_progress(progress_callback, sent_data, data_size);
  }
  return BLISP_OK;
}