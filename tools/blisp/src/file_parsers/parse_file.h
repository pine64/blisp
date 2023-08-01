#pragma once
#include <stdio.h>
#include <stdlib.h>
#include "parsed_firmware_file.h"

#define PARSED_ERROR_INVALID_FILETYPE -0x1000
#define PARSED_ERROR_CANT_OPEN_FILE -0x1001
#define PARSED_ERROR_TOO_BIG -0x1001 /* Input expands to be too big */
#define PARSED_ERROR_BAD_DFU -0x1002 /* DFU file provided but not valid */

// This attempts to parse the given file, and returns the parsed version of that
// file. This will handle any repacking required to create one contigious file
// Eg if the input file has holes,they will be 0x00 filled
// And headers etc are parsed to determine start position

int parse_firmware_file(const char* file_path_on_disk,
                        parsed_firmware_file_t* parsed_results);

// Internal util
ssize_t get_file_contents(const char* file_path_on_disk,
                          uint8_t** file_contents);