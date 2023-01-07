// SPDX-License-Identifier: MIT
#include "common.h"
#include <blisp.h>
#include <stdint.h>
#include <stdio.h>
#include <inttypes.h>
#include "util.h"

/**
 * Prepares chip to access flash
 * this means performing handshake, and loading eflash_loader if needed.
 */
int32_t blisp_prepare_flash(struct blisp_device* device) {
  int32_t ret = 0;
  FILE* eflash_loader_file = NULL;

  printf("Sending a handshake...");
  ret = blisp_device_handshake(device, false);
  if (ret != BLISP_OK) {
    fprintf(stderr, "\nFailed to handshake with device.\n");
    return -1;
  }
  printf(" OK\nGetting chip info...");
  struct blisp_boot_info boot_info;
  ret = blisp_device_get_boot_info(device, &boot_info);
  if (ret != BLISP_OK) {
    fprintf(stderr, "\nFailed to get boot info.\n");
    return -1;
  }

  if (boot_info.boot_rom_version[0] == 255 &&
      boot_info.boot_rom_version[1] == 255 &&
      boot_info.boot_rom_version[2] == 255 &&
      boot_info.boot_rom_version[3] == 255) {
    printf(" OK\nDevice already in eflash_loader.\n");
    return 0;
  }

  printf(
      " BootROM version %d.%d.%d.%d, ChipID: "
      "%02X%02X%02X%02X%02X%02X%02X%02X\n",
      boot_info.boot_rom_version[0], boot_info.boot_rom_version[1],
      boot_info.boot_rom_version[2], boot_info.boot_rom_version[3],
      boot_info.chip_id[0], boot_info.chip_id[1], boot_info.chip_id[2],
      boot_info.chip_id[3], boot_info.chip_id[4], boot_info.chip_id[5],
      boot_info.chip_id[6], boot_info.chip_id[7]);

  char exe_path[PATH_MAX];
  char eflash_loader_path[PATH_MAX];
  if (util_get_binary_folder(exe_path, PATH_MAX) <= 0) {
    fprintf(stderr,
            "Failed to find executable path to search for the "
            "eflash loader\n");
    return -1;
  }
  snprintf(eflash_loader_path, PATH_MAX, "%s/data/%s/eflash_loader_%s.bin",
           exe_path, device->chip->type_str, device->chip->default_xtal);
  printf("Loading the eflash loader file from disk\n");
  eflash_loader_file = fopen(eflash_loader_path, "rb");  // TODO: Error handling
  if (eflash_loader_file == NULL) {
    fprintf(stderr,
            "Could not open the eflash loader file from disk.\n"
            "Does \"%s\" exist?\n",
            eflash_loader_path);
    ret = -1;
    goto exit1;
  }
  uint8_t
      eflash_loader_header[176];  // TODO: Remap it to the boot header struct
  fread(eflash_loader_header, 176, 1,
        eflash_loader_file);  // TODO: Error handling

  printf("Loading eflash_loader...\n");
  ret = blisp_device_load_boot_header(device, eflash_loader_header);
  if (ret != BLISP_OK) {
    fprintf(stderr, "Failed to load boot header.\n");
    ret = -1;
    goto exit1;
  }

  {
    uint32_t sent_data = 0;
    uint32_t buffer_size = 0;
    uint8_t buffer[4092];

    // TODO: Real checking of segments count
    for (uint8_t seg_index = 0; seg_index < 1; seg_index++) {
      struct blisp_segment_header segment_header = {0};
      fread(&segment_header, 16, 1,
            eflash_loader_file);  // TODO: Error handling

      ret = blisp_device_load_segment_header(device, &segment_header);
      if (ret != 0) {
        fprintf(stderr, "Failed to load segment header.\n");
        ret = -1;
        goto exit1;
      }
      printf("Flashing %d. segment\n", seg_index + 1);
      printf("0b / %" PRIu32 "b (0.00%%)\n", segment_header.length);

      while (sent_data < segment_header.length) {
        buffer_size = segment_header.length - sent_data;
        if (buffer_size > 4092) {
          buffer_size = 4092;
        }
        fread(buffer, buffer_size, 1, eflash_loader_file);
        ret = blisp_device_load_segment_data(
            device, buffer, buffer_size);  // TODO: Error handling
        if (ret < BLISP_OK) {
          fprintf(stderr, "Failed to load segment data. (ret %d)\n", ret);
          ret = -1;
          goto exit1;
        }
        sent_data += buffer_size;
        printf("%" PRIu32 "b / %" PRIu32 "b (%.2f%%)\n", sent_data,
               segment_header.length,
               (((float)sent_data / (float)segment_header.length) * 100.0f));
      }
    }
  }

  ret = blisp_device_check_image(device);
  if (ret != BLISP_OK) {
    fprintf(stderr, "Failed to check image.\n");
    ret = -1;
    goto exit1;
  }

  ret = blisp_device_run_image(device);
  if (ret != BLISP_OK) {
    fprintf(stderr, "Failed to run image.\n");
    ret = -1;
    goto exit1;
  }

  printf("Sending a handshake...");
  ret = blisp_device_handshake(device, true);
  if (ret != BLISP_OK) {
    fprintf(stderr, "\nFailed to handshake with device.\n");
    ret = -1;
    goto exit1;
  }
  printf(" OK\n");
exit1:
  if (eflash_loader_file != NULL)
    fclose(eflash_loader_file);
  return ret;
}