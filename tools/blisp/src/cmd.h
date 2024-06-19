// SPDX-License-Identifier: MIT
#ifndef BLISP_CMD_H
#define BLISP_CMD_H

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
extern struct cmd cmd_run;
extern struct cmd cmd_iot;

#endif  // BLISP_CMD_H
