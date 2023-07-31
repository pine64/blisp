#pragma once
#include <stdint.h>
#include <stdio.h>

// Parsed firmware file is a generic struct that we parse from a user input
// firmware file This is used so that we can (relatively) seamlessly handle
// .bin, .hex and .def files

typedef struct {
  bool needs_boot_struct;  // If true, boot struct should be generated
  uint8_t* payload;        // The main firmware payload
  size_t payload_length;   // Size of the payload
  size_t payload_address;  // Start address of the payload
} parsed_firmware_file_t;
