// SPDX-License-Identifier: MIT
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "argtable3.h"
#include "cmd.h"

struct cmd* cmds[] = {&cmd_write, &cmd_iot};

static uint8_t cmds_count = sizeof(cmds) / sizeof(cmds[0]);

static struct arg_lit* help;
static struct arg_lit* version;
static struct arg_end* end;
static void* argtable[3];

int8_t args_init() {
  argtable[0] = help = arg_lit0(NULL, "help", "print this help and exit");
  argtable[1] = version =
      arg_lit0(NULL, "version", "print version information and exit");
  argtable[2] = end = arg_end(20);

  if (arg_nullcheck(argtable) != 0) {
    fprintf(stderr, "insufficient memory\n");
    return -1;
  }

  return 0;
}

void print_help() {
  puts("Usage:");
  for (uint8_t i = 0; i < cmds_count; i++) {
    fputs("  blisp", stdout);
    cmds[i]->args_print_syntax();
  }
  fputs("  blisp", stdout);
  arg_print_syntax(stdout, argtable, "\n");
}

int8_t args_parse_exec(int argc, char** argv) {
  int error = arg_parse(argc, argv, argtable);
  if (error == 0) {
    if (help->count) {
      print_help();
      return 1;
    } else if (version->count) {
      printf("blisp 0.0.1\n");
      printf("Copyright (C) 2023 Marek Kraus and PINE64 Community\n");
      return 1;
    }
  }
  return 0;
}

void args_free() {
  arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
}

int main(int argc, char** argv) {
  int exit_code = 0;

  if (args_init() != 0) {
    exit_code = -1;
    goto exit;
  }

  for (uint8_t i = 0; i < cmds_count; i++) {
    if (cmds[i]->args_init() != 0) {
      exit_code = -1;
      goto exit;
    }
  }

  if (args_parse_exec(argc, argv)) {
    goto exit;
  }

  uint8_t command_found = false;
  for (uint8_t i = 0; i < cmds_count; i++) {
    if (cmds[i]->args_parse_exec(argc, argv)) {
      command_found = true;
      break;
    }
  }

  if (!command_found) {
    print_help();
  }

exit:
  for (uint8_t i = 0; i < cmds_count; i++) {
    cmds[i]->args_free();
  }
  args_free();
  return exit_code;
}
