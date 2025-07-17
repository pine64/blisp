// SPDX-License-Identifier: MIT
#ifndef BLISP_CMD_H
#define BLISP_CMD_H

#if defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
#include <unistd.h>
#elif defined(_WIN32) || defined(WIN32) 
#include <io.h>
#define R_OK 4
#define access _access
#endif

#include <stdint.h>
#include "error_codes.h"
struct cmd {
  const char* name;
  blisp_return_t (*args_init)();
  blisp_return_t (*args_parse_exec)(int argc, char** argv);
  void (*args_print_syntax)();
  void (*args_free)();
};

extern struct cmd cmd_write;
extern struct cmd cmd_iot;

#endif  // BLISP_CMD_H
