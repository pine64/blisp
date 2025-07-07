#include "hex_file.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parse_file.h"

// Convert ASCII hex character to integer
static int hex_to_int(char c) {
  if (c >= '0' && c <= '9')
    return c - '0';
  if (c >= 'A' && c <= 'F')
    return c - 'A' + 10;
  if (c >= 'a' && c <= 'f')
    return c - 'a' + 10;
  return -1;
}

// Convert 2 ASCII hex characters to a byte
static int hex_byte_to_int(const char* str) {
  int high = hex_to_int(str[0]);
  int low = hex_to_int(str[1]);
  if (high < 0 || low < 0)
    return -1;
  return (high << 4) | low;
}

// Parse a single Intel HEX line into the record type and data
// Returns: Record Type on success, negative error code on failure
static int parse_hex_line(const char* line,
                          uint8_t* data_buffer,
                          uint32_t* address,
                          uint32_t* base_address,
                          uint32_t* max_address,
                          uint32_t min_address) {
  size_t len = strlen(line);

  // Line must start with ':' and have at least 11 characters (:BBAAAATTCC)
  if (line[0] != ':' || len < 11) {
    return HEX_PARSE_ERROR_INVALID_FORMAT;
  }

  // Extract record fields
  int byte_count = hex_byte_to_int(line + 1);
  if (byte_count < 0)
    return HEX_PARSE_ERROR_INVALID_FORMAT;

  // Make sure line is long enough
  if (len < (size_t)(11 + byte_count * 2)) {
    return HEX_PARSE_ERROR_INVALID_FORMAT;
  }

  int addr_high = hex_byte_to_int(line + 3);
  int addr_low = hex_byte_to_int(line + 5);
  if (addr_high < 0 || addr_low < 0)
    return HEX_PARSE_ERROR_INVALID_FORMAT;

  uint16_t record_address = (addr_high << 8) | addr_low;

  int record_type = hex_byte_to_int(line + 7);
  if (record_type < 0)
    return HEX_PARSE_ERROR_INVALID_FORMAT;

  // Verify checksum
  uint8_t checksum = 0;
  for (int i = 1; i < 9 + byte_count * 2; i += 2) {
    int value = hex_byte_to_int(line + i);
    if (value < 0) {
      return HEX_PARSE_ERROR_INVALID_FORMAT;
    }
    checksum += value;
  }

  int file_checksum = hex_byte_to_int(line + 9 + byte_count * 2);
  if (file_checksum < 0) {
    return HEX_PARSE_ERROR_INVALID_FORMAT;
  }
  checksum = ~checksum + 1;  // Two's complement
  // Verify checksum
  if (checksum != file_checksum) {
    return HEX_PARSE_ERROR_CHECKSUM;
  }

  // Process the record based on record type
  switch (record_type) {
    case HEX_RECORD_DATA: {
      uint32_t absolute_address = *base_address + record_address;
      *address = absolute_address;

      // Update max address if this data extends beyond current max
      if (absolute_address + byte_count > *max_address) {
        *max_address = absolute_address + byte_count;
      }

      // Parse data bytes
      for (int i = 0; i < byte_count; i++) {
        int value = hex_byte_to_int(line + 9 + i * 2);
        if (value < 0)
          return HEX_PARSE_ERROR_INVALID_FORMAT;

        // Make sure we don't write outside our buffer
        if (data_buffer != NULL) {
          size_t buf_offset = absolute_address - min_address + i;
          data_buffer[buf_offset] = value;
        }
      }
      break;
    }

    case HEX_RECORD_EOF:
      // End of file, nothing to do
      break;

    case HEX_RECORD_EXTENDED_SEGMENT:
      // Set segment base address (offset = value * 16)
      if (byte_count != 2)
        return HEX_PARSE_ERROR_INVALID_FORMAT;
      int value_high = hex_byte_to_int(line + 9);
      int value_low = hex_byte_to_int(line + 11);
      if (value_high < 0 || value_low < 0)
        return HEX_PARSE_ERROR_INVALID_FORMAT;
      *base_address = ((value_high << 8) | value_low) << 4;
      break;

    case HEX_RECORD_EXTENDED_LINEAR:
      // Set high-order 16 bits of address
      if (byte_count != 2)
        return HEX_PARSE_ERROR_INVALID_FORMAT;
      value_high = hex_byte_to_int(line + 9);
      value_low = hex_byte_to_int(line + 11);
      if (value_high < 0 || value_low < 0)
        return HEX_PARSE_ERROR_INVALID_FORMAT;
      *base_address = ((value_high << 8) | value_low) << 16;
      break;

    case HEX_RECORD_START_LINEAR:
      // Start linear address - store as a potential entry point
      // but not crucial for firmware extraction
      break;

    case HEX_RECORD_START_SEGMENT:
      // Start segment address - similar to above
      break;

    default:
      return HEX_PARSE_ERROR_UNSUPPORTED_RECORD;
  }

  return record_type;
}

int hex_file_parse(const char* file_path_on_disk,
                   uint8_t** payload,
                   size_t* payload_length,
                   size_t* payload_address) {
  FILE* file = fopen(file_path_on_disk, "r");
  if (!file) {
    fprintf(stderr, "Could not open file %s for reading\n", file_path_on_disk);
    return PARSED_ERROR_CANT_OPEN_FILE;
  }

  // First pass: Find start and end addresses, and thus size of the payload
  char line[512];
  uint32_t base_address = 0;
  uint32_t address = 0;
  uint32_t min_address = 0xFFFFFFFF;
  uint32_t max_address = 0;
  bool found_data = false;

  // First pass to determine memory range
  while (fgets(line, sizeof(line), file)) {
    size_t len = strlen(line);
    // Trim trailing newline and carriage return
    while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r')) {
      line[--len] = 0;
    }
    if (len == 0 || line[0] != ':')
      continue;

    int result =
        parse_hex_line(line, NULL, &address, &base_address, &max_address, 0);
    if (result < 0) {
      fclose(file);
      return result;
    }

    // Check if this is a data record (type 0)
    if (result == HEX_RECORD_DATA) {
      found_data = true;
      if (address < min_address) {
        min_address = address;
      }
    }

    // If we hit EOF record, we're done
    if (result == HEX_RECORD_EOF) {
      break;
    }
  }

  // If no data was found, return error
  if (!found_data) {
    fclose(file);
    return HEX_PARSE_ERROR_INVALID_FORMAT;
  }

  // Calculate payload size
  size_t size = max_address - min_address;
  if (size > (1024 * 1024 * 128)) {  // Limit to 128 MB
    fclose(file);
    return HEX_PARSE_ERROR_TOO_LARGE;
  }
  // Allocate memory for the payload
  *payload = (uint8_t*)calloc(size, sizeof(uint8_t));
  if (!*payload) {
    fclose(file);
    return -1;
  }

  // Clear the memory to ensure all bytes are initialized
  memset(*payload, 0, size);

  // Second pass: actually parse the data and fill out the buffer with the data
  rewind(file);
  base_address = 0;

  while (fgets(line, sizeof(line), file)) {
    size_t len = strlen(line);
    // Trim trailing newline and carriage return
    while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r')) {
      line[--len] = 0;
    }
    if (len == 0 || line[0] != ':')
      continue;

    // When parsing for real, data is written to the payload buffer
    // with addresses relative to min_address
    uint32_t dummy_max = 0;

    int result = parse_hex_line(line, *payload, &address, &base_address,
                                &dummy_max, min_address);
    if (result < 0) {
      free(*payload);
      *payload = NULL;
      fclose(file);
      return result;
    }

    // If we hit EOF record, we're done
    if (result == HEX_RECORD_EOF) {
      break;
    }
  }

  fclose(file);

  // Set output parameters
  *payload_length = size;
  *payload_address = min_address;

  return 0;
}
