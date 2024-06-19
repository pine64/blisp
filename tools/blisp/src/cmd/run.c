#include <argtable3.h>
#include <blisp_easy.h>
#include "../cmd.h"
#include "../common.h"

#define REG_EXTENDED 1
#define REG_ICASE (REG_EXTENDED << 1)

static struct arg_rex* cmd;
static struct arg_str *port_name, *chip_type;  // TODO: Make this common
static struct arg_lit* reset;
static struct arg_end* end;
static struct arg_file* binary_to_run;
static void* cmd_run_argtable[6];


blisp_return_t cmd_run_args_init() {
  cmd_run_argtable[0] = cmd =
      arg_rex1(NULL, NULL, "run", NULL, REG_ICASE, NULL);
  cmd_run_argtable[1] = chip_type =
      arg_str1("c", "chip", "<chip_type>", "Chip Type");
  cmd_run_argtable[2] = port_name =
      arg_str0("p", "port", "<port_name>",
               "Name/Path to the Serial Port (empty for search)");
  cmd_run_argtable[3] = reset =
      arg_lit0(NULL, "reset", "Reset chip after write");
  cmd_run_argtable[4] = binary_to_run =
      arg_file1(NULL, NULL, "<input>", "Binary to run");
  cmd_run_argtable[5] = end = arg_end(10);

  if (arg_nullcheck(cmd_run_argtable) != 0) {
    fprintf(stderr, "insufficient memory\n");
    return BLISP_ERR_OUT_OF_MEMORY;
  }
  return BLISP_OK;
}

void cmd_run_args_print_glossary() {
  fputs("Usage: blisp", stdout);
  arg_print_syntax(stdout, cmd_run_argtable, "\n");
  puts("Flashes firmware to RAM and then executes it.");
  arg_print_glossary(stdout, cmd_run_argtable, "  %-25s %s\n");
}

blisp_return_t blisp_run_firmware() {
  struct blisp_device device;
  blisp_return_t ret;

  ret = blisp_common_init_device(&device, port_name, chip_type);
  if (ret != BLISP_OK) {
    return ret;
  }

  FILE* data_file = fopen(binary_to_run->filename[0], "rb");
  if (data_file == NULL) {
    fprintf(stderr, "Failed to open data file \"%s\".\n",
            binary_to_run->filename[0]);
    ret = BLISP_ERR_CANT_OPEN_FILE;
    goto exit1;
  }

  if (blisp_common_prepare_flash(&device, false) != 0) {
    // TODO: Error handling
    goto exit1;
  }

  struct blisp_easy_transport firmware_transport =
      blisp_easy_transport_new_from_file(data_file);

  ret = blisp_easy_load_ram_app(&device, &firmware_transport,
                                blisp_common_progress_callback);
  if (ret != BLISP_OK) {
    fprintf(stderr, "Failed to load firmware, ret: %d\n", ret);
    goto exit1;
  }

  ret = blisp_device_check_image(&device);
  if (ret != 0) {
    fprintf(stderr, "Failed to check image.\n");
    goto exit1;
  }

  ret = blisp_device_run_image(&device);
  if (ret != BLISP_OK) {
    fprintf(stderr, "Failed to run image.\n");
    goto exit1;
  }

exit2:
  if (data_file != NULL)
    fclose(data_file);
exit1:
  blisp_device_close(&device);
  return ret;
}

blisp_return_t cmd_run_parse_exec(int argc, char** argv) {
  int errors = arg_parse(argc, argv, cmd_run_argtable);
  if (errors == 0) {
    blisp_run_firmware();
    return BLISP_OK;
  } else if (cmd->count == 1) {
    cmd_run_args_print_glossary();
    return BLISP_OK;
  }
  return BLISP_ERR_INVALID_COMMAND;
}

void cmd_run_args_print_syntax() {
  arg_print_syntax(stdout, cmd_run_argtable, "\n");
}

void cmd_run_free() {
  arg_freetable(cmd_run_argtable,
                sizeof(cmd_run_argtable) / sizeof(cmd_run_argtable[0]));
}

struct cmd cmd_run = {"run", cmd_run_args_init, cmd_run_parse_exec,
                      cmd_run_args_print_syntax, cmd_run_free};
