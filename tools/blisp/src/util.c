// SPDX-License-Identifier: MIT
#include "util.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#ifdef __linux__
#include <unistd.h>
#elif defined(_MSC_VER)
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#include <windows.h>
#define PATH_MAX MAX_PATH
#elif defined(__APPLE__)
#include <sys/syslimits.h>
#include <assert.h>
#endif

#ifdef __APPLE__
// Ugh. This stuff is just so messy without C++17 or Qt...
// These are not thread safe, but it doesn't place the responsibility
// to free an allocated buffer on the caller.nn

static void util_get_executable_path(char* buffer_out, uint32_t max_size) {
  assert(max_size >= PATH_MAX);  // n.b. 1024 on MacOS. 4K on most Linux.

  char raw_path_name[PATH_MAX];   // $HOME/../../var/tmp/x
  char real_path_name[PATH_MAX];  // /var/tmp/x
  uint32_t raw_path_size = sizeof(raw_path_name);

  if (!_NSGetExecutablePath(raw_path_name, &raw_path_size)) {
    realpath(raw_path_name, real_path_name);
  }
  // *real_path_name  is appropriately sized and null terminated.
  strcpy(buffer_out, real_path_name);
}
#endif

ssize_t util_get_binary_folder(char* buffer, uint32_t buffer_size) {
#ifdef __linux__
  if (readlink("/proc/self/exe", buffer, buffer_size) <= 0) {
    return -1;
  }
  char* pos = strrchr(buffer, '/');
#elif defined(__APPLE__)
  util_get_executable_path(buffer, buffer_size);
  char* pos = strrchr(buffer, '/');
#else
  if (GetModuleFileName(NULL, buffer, buffer_size) <= 0) {
    return -1;
  }
  char* pos = strrchr(buffer, '\\');
#endif
  pos[0] = '\0';
  return pos - buffer;
}