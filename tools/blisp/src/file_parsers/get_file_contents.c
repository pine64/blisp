#include <stdio.h>
#include <stdlib.h>
#include "parse_file.h"
// Returns file size _or_ negative on error
ssize_t get_file_contents(const char* file_path_on_disk,
                          uint8_t** file_contents) {
  size_t read_count;
  size_t file_size = 0;
  size_t read_total = 0;

  FILE* f;

  if (file_contents == NULL) {
    return -99;
  }

  f = fopen(file_path_on_disk, "rb");
  if (f <= 0) {
    fprintf(stderr, "Could not open file %s for reading\r\n",
            file_path_on_disk);
    return -1;
  }

  fseek(f, 0, SEEK_END);
  file_size = ftell(f);
  fseek(f, 0, SEEK_SET);
  *file_contents = calloc(file_size, sizeof(uint8_t));

  while (read_total < file_size) {
    size_t to_read = file_size - read_total;
    /* read() limit on Linux, slightly below MAX_INT on Windows */
    if (to_read > 0x7ffff000)
      to_read = 0x7ffff000;
    read_count = fread((*file_contents) + read_total, 1, to_read, f);
    if (read_count == 0)
      break;
    // If error and not end of file, break
    if (read_count == -1 && !feof(f))
      break;
    read_total += read_count;
  }
  if (read_total != file_size) {
    fprintf(stderr, "Could only read %lld of %lld bytes from %s",
            (long long)read_total, (long long)file_size, file_path_on_disk);
    return -1;
  }
  fclose(f);
  return (ssize_t)file_size;
}