#include <argtable3.h>
#include <blisp_easy.h>
#include "../cmd.h"
#include "../common.h"

#define REG_EXTENDED 1
#define REG_ICASE (REG_EXTENDED << 1)

static struct arg_rex* cmd;
static struct arg_file* single_download;
static struct arg_int* single_download_location;
static struct arg_str *port_name, *chip_type;  // TODO: Make this common
static struct arg_int *baudrate;
static struct arg_lit* reset;
static struct arg_end* end;
static void* cmd_iot_argtable[8];

blisp_return_t blisp_single_download(void) {
  struct blisp_device device;
  blisp_return_t ret;

  uint32_t baud = DEFAULT_BAUDRATE;
  if (baudrate->count == 1) {
    if (*baudrate->ival < 0) {
      fprintf(stderr, "Baud rate cannot be negative!\n");
      return BLISP_ERR_INVALID_COMMAND;
    } else {
      baud = *baudrate->ival;
    }
  }

  ret = blisp_common_init_device(&device, port_name, chip_type, baud);

  if (ret != BLISP_OK) {
    return ret;
  }
  ret = blisp_common_prepare_flash(&device);
  if (ret != BLISP_OK) {
    // TODO: Error handling
    goto exit1;
  }

  FILE* data_file = fopen(single_download->filename[0], "rb");
  if (data_file == NULL) {
    fprintf(stderr, "Failed to open data file \"%s\".\n",
            single_download->filename[0]);
    ret = BLISP_ERR_CANT_OPEN_FILE;
    goto exit1;
  }
  fseek(data_file, 0, SEEK_END);
  int64_t data_file_size = ftell(data_file);
  rewind(data_file);

  printf("Erasing the area, this might take a while...\n");
  ret = blisp_device_flash_erase(
      &device, *single_download_location->ival,
      *single_download_location->ival + data_file_size - 1);
  if (ret != BLISP_OK) {
    fprintf(stderr, "Failed to erase.\n");
    goto exit2;
  }

  printf("Writing the data...\n");
  struct blisp_easy_transport data_transport =
      blisp_easy_transport_new_from_file(data_file);

  ret = blisp_easy_flash_write(&device, &data_transport,
                               *single_download_location->ival, data_file_size,
                               blisp_common_progress_callback);
  if (ret < BLISP_OK) {
    fprintf(stderr, "Failed to write data to flash.\n");
    goto exit2;
  }

  printf("Checking program...\n");
  ret = blisp_device_program_check(&device);
  if (ret != BLISP_OK) {
    fprintf(stderr, "Failed to check program.\n");
    goto exit2;
  }
  printf("Program OK!\n");

  if (reset->count > 0) {  // TODO: could be common
    printf("Resetting the chip.\n");
    ret = blisp_device_reset(&device);
    if (ret != BLISP_OK) {
      fprintf(stderr, "Failed to reset chip.\n");
      goto exit2;
    }
  }
  if (ret == BLISP_OK) {
    printf("Download complete!\n");
  }

exit2:
  if (data_file != NULL)
    fclose(data_file);
exit1:
  blisp_device_close(&device);

  return ret;
}

blisp_return_t cmd_iot_args_init() {
  cmd_iot_argtable[0] = cmd =
      arg_rex1(NULL, NULL, "iot", NULL, REG_ICASE, NULL);
  cmd_iot_argtable[1] = chip_type =
      arg_str1("c", "chip", "<chip_type>", "Chip Type");
  cmd_iot_argtable[2] = port_name =
      arg_str0("p", "port", "<port_name>",
               "Name/Path to the Serial Port (empty for search)");
  cmd_iot_argtable[3] = baudrate =
      arg_int0("b", "baudrate", "<baud rate>",
               "Serial baud rate (default: " XSTR(DEFAULT_BAUDRATE) ")");
  cmd_iot_argtable[4] = reset =
      arg_lit0(NULL, "reset", "Reset chip after write");
  cmd_iot_argtable[5] = single_download =
      arg_file0("s", "single-down", "<file>", "Single download file");
  cmd_iot_argtable[6] = single_download_location =
      arg_int0("l", "single-down-loc", NULL, "Single download offset");
  cmd_iot_argtable[7] = end = arg_end(10);

  if (arg_nullcheck(cmd_iot_argtable) != 0) {
    fprintf(stderr, "insufficient memory\n");
    return BLISP_ERR_OUT_OF_MEMORY;
  }
  return BLISP_OK;
}

void cmd_iot_args_print_glossary() {
  fputs("Usage: blisp", stdout);
  arg_print_syntax(stdout, cmd_iot_argtable, "\n");
  puts("Flashes firmware as Bouffalo's DevCube");
  arg_print_glossary(stdout, cmd_iot_argtable, "  %-25s %s\n");
}

blisp_return_t cmd_iot_parse_exec(int argc, char** argv) {
  int errors = arg_parse(argc, argv, cmd_iot_argtable);
  if (errors == 0) {
    if (single_download->count == 1 && single_download_location->count == 1) {
      return blisp_single_download();
    } else {
      return BLISP_ERR_INVALID_COMMAND;
    }
  } else if (cmd->count == 1) {
    cmd_iot_args_print_glossary();
    return BLISP_OK;
  }
  return BLISP_ERR_INVALID_COMMAND;
}

void cmd_iot_args_print_syntax() {
  arg_print_syntax(stdout, cmd_iot_argtable, "\n");
}

void cmd_iot_free() {
  arg_freetable(cmd_iot_argtable,
                sizeof(cmd_iot_argtable) / sizeof(cmd_iot_argtable[0]));
}

struct cmd cmd_iot = {"iot", cmd_iot_args_init, cmd_iot_parse_exec,
                      cmd_iot_args_print_syntax, cmd_iot_free};
