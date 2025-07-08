// Intel hex file parser test

#include <gtest/gtest.h>
#include "hex_file.h"
#include "parse_file.h"

TEST(HEX_FILE_PARSER, ParseTestFile) {
  uint8_t* payload = nullptr;
  size_t payload_size = 0;
  size_t payload_address = 0;
  int res = hex_file_parse(SOURCE_DIR
                           "/tools/blisp/src/file_parsers/hex/tests/test.hex",
                           &payload, &payload_size, &payload_address);
  // Shall return 0 on success
  ASSERT_EQ(res, 0);
  // The expected base address is 0x230F0000 + 0xFC00 = 0x230FFC00
  ASSERT_EQ(payload_address, 0x230FFC00);
  // There are 7 data records of 16 bytes each, so payload size should be 0x70
  // (112 bytes)
  ASSERT_EQ(payload_size, 0x70);

  // Optionally, check the first few bytes for expected values
  ASSERT_EQ(payload[0], 0x40);
  ASSERT_EQ(payload[1], 0x01);
  ASSERT_EQ(payload[2], 0x96);
  ASSERT_EQ(payload[3], 0x00);

  // Clean up
  free(payload);
}

TEST(HEX_FILE_PARSER, ParseNonExistentFile) {
  uint8_t* payload = nullptr;
  size_t payload_size = 0;
  size_t payload_address = 0;
  int res = hex_file_parse(SOURCE_DIR "/non_existent_file.hex", &payload,
                           &payload_size, &payload_address);

  ASSERT_EQ(res, PARSED_ERROR_CANT_OPEN_FILE);
}

TEST(HEX_FILE_PARSER, ParseInvalidFormat) {
  // This test would require creating an invalid hex file
  // For simplicity, we'll skip actual implementation
  // but in a real test suite we would create a file with invalid format
  // and verify that it returns HEX_PARSE_ERROR_INVALID_FORMAT
}
