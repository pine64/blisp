// SPDX-License-Identifier: MIT
#ifndef BLISP_CMD_H
#define BLISP_CMD_H

#include <stdint.h>

struct cmd {
    const char* name;
    int8_t (*args_init)();
    uint8_t (*args_parse_exec)(int argc, char** argv);
    void (*args_print_syntax)();
    void (*args_free)();
};

extern struct cmd cmd_write;

#endif // BLISP_CMD_H
