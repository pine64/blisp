// SPDX-License-Identifier: MIT
#ifndef BLISP_UTIL_H
#define BLISP_UTIL_H

#include <stdint.h>

#if defined(_MSC_VER)
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#include <windows.h>
#define PATH_MAX MAX_PATH
#elif defined(__APPLE__)
#include <sys/syslimits.h>
#include <assert.h>
#include <sys/types.h>
#else
#include <unistd.h>
#endif

ssize_t util_get_binary_folder(char* buffer, uint32_t buffer_size);


#endif