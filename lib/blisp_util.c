// SPDX-License-Identifier: MIT
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>

#ifdef WIN32
#  include <windows.h>
#else
#  include <time.h>
#endif

#include "blisp_util.h"

void blisp_dlog(const char* format, ...)
{
  fflush(stdout);
  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);
  fputc('\n', stderr);
}


void sleep_ms(int milliseconds) {
#ifdef WIN32
  Sleep(milliseconds);
#else
  struct timespec ts;
  ts.tv_sec = milliseconds / 1000;
  ts.tv_nsec = (milliseconds % 1000) * 1000000;
  nanosleep(&ts, NULL);
#endif
}

uint32_t crc32_calculate(const void *data, size_t data_len)
{
  uint32_t crc = 0xffffffff;
  const unsigned char *d = (const unsigned char *)data;
  unsigned int tbl_idx;

  while (data_len--) {
    tbl_idx = (crc ^ *d) & 0xff;
    crc = (crc_table[tbl_idx] ^ (crc >> 8)) & 0xffffffff;
    d++;
  }
  return (crc & 0xffffffff) ^ 0xffffffff;
}
