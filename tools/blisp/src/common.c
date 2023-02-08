// SPDX-License-Identifier: MIT
#include "common.h"
#include <argtable3.h>
#include <blisp.h>
#include <inttypes.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "blisp_easy.h"
#include "util.h"

void blisp_common_progress_callback(uint32_t current_value, uint32_t max_value) {
  printf("%" PRIu32 "b / %u (%.2f%%)\n", current_value, max_value,
         (((float)current_value / (float)max_value) * 100.0f));
}

int32_t blisp_common_init_device(struct blisp_device* device, struct arg_str* port_name, struct arg_str* chip_type)
{
  if (chip_type->count == 0) {
    fprintf(stderr, "Chip type is invalid.\n");
    return -1;
  }

  struct blisp_chip* chip = NULL;

  if (strcmp(chip_type->sval[0], "bl70x") == 0) {
    chip = &blisp_chip_bl70x;
  } else if (strcmp(chip_type->sval[0], "bl60x") == 0) {
    chip = &blisp_chip_bl60x;
  } else {
    fprintf(stderr, "Chip type is invalid.\n");
    return -1;
  }

  int32_t ret;
  ret = blisp_device_init(device, chip);
  if (ret != BLISP_OK) {
    fprintf(stderr, "Failed to init device.\n");
    return -1;
  }
  ret = blisp_device_open(device,
                          port_name->count == 1 ? port_name->sval[0] : NULL);
  if (ret != BLISP_OK) {
    fprintf(stderr, ret == BLISP_ERR_DEVICE_NOT_FOUND ? "Device not found\n" : "Failed to open device.\n");
    return -1;
  }

  return 0;
}

/**
 * Prepares chip to access flash
 * this means performing handshake, and loading eflash_loader if needed.
 */
int32_t blisp_common_prepare_flash(struct blisp_device* device) {
  int32_t ret = 0;

  printf("Sending a handshake...\n");
  ret = blisp_device_handshake(device, false);
  if (ret != BLISP_OK) {
    fprintf(stderr, "Failed to handshake with device.\n");
    return -1;
  }
  printf("Handshake successful!\nGetting chip info...\n");
  struct blisp_boot_info boot_info;
  ret = blisp_device_get_boot_info(device, &boot_info);
  if (ret != BLISP_OK) {
    fprintf(stderr, "Failed to get boot info.\n");
    return -1;
  }

  printf(
      "BootROM version %d.%d.%d.%d, ChipID: "
      "%02X%02X%02X%02X%02X%02X%02X%02X\n",
      boot_info.boot_rom_version[0], boot_info.boot_rom_version[1],
      boot_info.boot_rom_version[2], boot_info.boot_rom_version[3],
      boot_info.chip_id[0], boot_info.chip_id[1], boot_info.chip_id[2],
      boot_info.chip_id[3], boot_info.chip_id[4], boot_info.chip_id[5],
      boot_info.chip_id[6], boot_info.chip_id[7]);

  if (device->chip->load_eflash_loader == NULL) {
    return 0;
  }

  if (boot_info.boot_rom_version[0] == 255 &&
      boot_info.boot_rom_version[1] == 255 &&
      boot_info.boot_rom_version[2] == 255 &&
      boot_info.boot_rom_version[3] == 255) {
    printf("Device already in eflash_loader.\n");
    return 0;
  }

  uint8_t* eflash_loader_buffer = NULL;
  // TODO: Error check
  int64_t eflash_loader_buffer_length = device->chip->load_eflash_loader(0, &eflash_loader_buffer);

  struct blisp_easy_transport eflash_loader_transport =
      blisp_easy_transport_new_from_memory(eflash_loader_buffer, eflash_loader_buffer_length);

  ret = blisp_easy_load_ram_app(device, &eflash_loader_transport, blisp_common_progress_callback);

  if (ret != BLISP_OK) {
    fprintf(stderr, "Failed to load eflash_loader, ret: %d\n", ret);
    ret = -1;
    goto exit1;
  }

  free(eflash_loader_buffer);
  eflash_loader_buffer = NULL;

  ret = blisp_device_check_image(device);
  if (ret != 0) {
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

  printf("Sending a handshake...\n");
  ret = blisp_device_handshake(device, true);
  if (ret != BLISP_OK) {
    fprintf(stderr, "Failed to handshake with device.\n");
    ret = -1;
    goto exit1;
  }
  printf("Handshake with eflash_loader successful.\n");
exit1:
  if (eflash_loader_buffer != NULL)
    free(eflash_loader_buffer);
  return ret;
}