//
// Created for Intel HEX file parsing
//

#ifndef BLISP_HEX_FILE_H
#define BLISP_HEX_FILE_H

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

// Error codes specific to hex parsing
#define HEX_PARSE_ERROR_INVALID_FORMAT -0x2001
#define HEX_PARSE_ERROR_CHECKSUM -0x2002
#define HEX_PARSE_ERROR_UNSUPPORTED_RECORD -0x2003
#define HEX_PARSE_ERROR_TOO_LARGE -0x2004

// Intel HEX record types
typedef enum {
  HEX_RECORD_DATA = 0x00,              // Data record
  HEX_RECORD_EOF = 0x01,               // End of file record
  HEX_RECORD_EXTENDED_SEGMENT = 0x02,  // Extended segment address record
  HEX_RECORD_START_SEGMENT = 0x03,     // Start segment address record
  HEX_RECORD_EXTENDED_LINEAR = 0x04,   // Extended linear address record
  HEX_RECORD_START_LINEAR = 0x05       // Start linear address record
} hex_record_type_t;

// Parse an Intel HEX file and return a contiguous memory block
// Parameters:
//   file_path_on_disk: Path to the Intel HEX file
//   payload: Pointer to the buffer that will hold the parsed data
//   payload_length: Size of the payload
//   payload_address: Start address of the payload
// Returns:
//   0 on success, negative value on error
int hex_file_parse(const char* file_path_on_disk,
                   uint8_t** payload,
                   size_t* payload_length,
                   size_t* payload_address);

#ifdef __cplusplus
};
#endif

#endif  // BLISP_HEX_FILE_H
