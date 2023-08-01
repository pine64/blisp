#include "parse_file.h"
#include <string.h>
#include "bin_file.h"
#include "dfu_file.h"

const char* get_filename_ext(const char* filename) {
  const char* dot = strrchr(filename, '.');
  if (!dot || dot == filename)
    return "";
  return dot + 1;
}

int parse_firmware_file(const char* file_path_on_disk,
                        parsed_firmware_file_t* parsed_results) {
  // Switchcase on the extension of the file
  const char* ext = get_filename_ext(file_path_on_disk);
  int res = PARSED_ERROR_INVALID_FILETYPE;
  if (strncmp(ext, "dfu", 3) == 0 || strncmp(ext, "DFU", 3) == 0) {
    printf("Input file identified as a .dfu file\n");
    // Handle as a .dfu file
    res = dfu_file_parse(file_path_on_disk, &parsed_results->payload,
                         &parsed_results->payload_length,
                         &parsed_results->payload_address);
  } else if (strncmp(ext, "bin", 3) == 0 || strncmp(ext, "BIN", 3) == 0) {
    printf("Input file identified as a .bin file\n");
    // Raw binary file
    res = bin_file_parse(file_path_on_disk, &parsed_results->payload,
                         &parsed_results->payload_length,
                         &parsed_results->payload_address);
  }
  // If we wanted to support hex files, here would be where

  // If the firmware starts at "0" we need to pre-pend a boot sector later on
  parsed_results->needs_boot_struct = parsed_results->payload_address == 0;

  return res;
}