#include <stdio.h>
#include "parse_file.h"

int bin_file_parse(const char* file_path_on_disk,
                   uint8_t** payload,
                   size_t* payload_length,
                   size_t* payload_address) {
  // Bin files a dumb so we cant do any fancy logic
  *payload_address = 0;  // We cant know otherwise
  ssize_t len = get_file_contents(file_path_on_disk, payload);
  if (len > 0) {
    *payload_length = len;
  }
  return len;
}