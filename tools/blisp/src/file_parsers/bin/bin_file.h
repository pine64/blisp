//
// Created by ralim on 01/08/23.
//

#ifndef BLISP_BIN_FILE_H
#define BLISP_BIN_FILE_H

#include <malloc.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

int bin_file_parse(const char* file_path_on_disk,
                   uint8_t** payload,
                   size_t* payload_length,
                   size_t* payload_address);
#ifdef __cplusplus
};
#endif

#endif  // BLISP_BIN_FILE_H
