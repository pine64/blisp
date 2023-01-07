// SPDX-License-Identifier: MIT
#ifndef BLISP_UTIL_H
#define BLISP_UTIL_H

#include <unistd.h>
#include <stdint.h>

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

#ifdef __linux__
#include <linux/limits.h>
#endif

ssize_t util_get_binary_folder(char* buffer, uint32_t buffer_size);


#endif