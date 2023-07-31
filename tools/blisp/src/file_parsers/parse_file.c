#include "parse_file.h"
#include <string.h>
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

  if (strncmp(ext, "dfu", 3) == 0 || strncmp(ext, "DFU", 3) == 0) {
    printf("Input file identified as a .dfu file\r\n");
    // Handle as a .dfu file
    return dfu_file_parse("test.dfu", &parsed_results->payload,
                          &parsed_results->payload_length,
                          &parsed_results->payload_address);
  } else if (strncmp(ext, "bin", 3) == 0 || strncmp(ext, "BIN", 3) == 0) {
    printf("Input file identified as a .bin file\r\n");
    // Raw binary file
  }
  // TODO: Hex files?
  return PARSED_ERROR_INVALID_FILETYPE;
}