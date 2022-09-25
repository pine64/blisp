//
// Created by ralim on 26/09/22.
//

#ifndef BLISP_DFU_FILE_H
#define BLISP_DFU_FILE_H

#include <malloc.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

int dfu_file_parse(const char* file_path_on_disk, uint8_t** payload,
                   size_t* payload_length, size_t* payload_address);
// Internal

uint32_t crc32_byte(uint32_t accum, uint8_t delta);
#ifdef __cplusplus
};
#endif

#endif // BLISP_DFU_FILE_H
