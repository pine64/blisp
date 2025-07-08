//
// Created by ralim on 26/09/22.
//

#include <gtest/gtest.h>
#include "dfu_file.h"
TEST(DFU_FILE_PARSER, ParseTestFile) {
  uint8_t* payload = nullptr;
  size_t payload_size = 0;
  size_t payload_address = 0;
  int res = dfu_file_parse(SOURCE_DIR
                           "/tools/blisp/src/file_parsers/dfu/tests/test.dfu",
                           &payload, &payload_size, &payload_address);
  ASSERT_EQ(res, 1);
  ASSERT_EQ(payload_size, 1337);
  ASSERT_EQ(payload_address, 0x11223344);
}
