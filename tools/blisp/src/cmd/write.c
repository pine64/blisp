// SPDX-License-Identifier: MIT
#include <argtable3.h>
#include <blisp.h>
#include <blisp_easy.h>
#include <blisp_struct.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include "../cmd.h"
#include "../common.h"
#include "../util.h"
#include "parse_file.h"

#define REG_EXTENDED 1
#define REG_ICASE (REG_EXTENDED << 1)

static struct arg_rex* cmd;
static struct arg_file* binary_to_write;
static struct arg_str *port_name, *chip_type;
static struct arg_int *baudrate;
static struct arg_lit* reset;
static struct arg_end* end;
static void* cmd_write_argtable[7];

void fill_up_boot_header(struct bfl_boot_header* boot_header) {
  memcpy(boot_header->magiccode, "BFNP", 4);

  boot_header->revison = 0x01;
  memcpy(boot_header->flashCfg.magiccode, "FCFG", 4);
  boot_header->flashCfg.cfg.ioMode = 0x11;
  boot_header->flashCfg.cfg.cReadSupport = 0x00;
  boot_header->flashCfg.cfg.clkDelay = 0x01;
  boot_header->flashCfg.cfg.clkInvert = 0x01;
  boot_header->flashCfg.cfg.resetEnCmd = 0x66;
  boot_header->flashCfg.cfg.resetCmd = 0x99;
  boot_header->flashCfg.cfg.resetCreadCmd = 0xFF;
  boot_header->flashCfg.cfg.resetCreadCmdSize = 0x03;
  boot_header->flashCfg.cfg.jedecIdCmd = 0x9F;
  boot_header->flashCfg.cfg.jedecIdCmdDmyClk = 0x00;
  boot_header->flashCfg.cfg.qpiJedecIdCmd = 0x9F;
  boot_header->flashCfg.cfg.qpiJedecIdCmdDmyClk = 0x00;
  boot_header->flashCfg.cfg.sectorSize = 0x04;
  boot_header->flashCfg.cfg.mid = 0xC2;
  boot_header->flashCfg.cfg.pageSize = 0x100;
  boot_header->flashCfg.cfg.chipEraseCmd = 0xC7;
  boot_header->flashCfg.cfg.sectorEraseCmd = 0x20;
  boot_header->flashCfg.cfg.blk32EraseCmd = 0x52;
  boot_header->flashCfg.cfg.blk64EraseCmd = 0xD8;
  boot_header->flashCfg.cfg.writeEnableCmd = 0x06;
  boot_header->flashCfg.cfg.pageProgramCmd = 0x02;
  boot_header->flashCfg.cfg.qpageProgramCmd = 0x32;
  boot_header->flashCfg.cfg.qppAddrMode = 0x00;
  boot_header->flashCfg.cfg.fastReadCmd = 0x0B;
  boot_header->flashCfg.cfg.frDmyClk = 0x01;
  boot_header->flashCfg.cfg.qpiFastReadCmd = 0x0B;
  boot_header->flashCfg.cfg.qpiFrDmyClk = 0x01;
  boot_header->flashCfg.cfg.fastReadDoCmd = 0x3B;
  boot_header->flashCfg.cfg.frDoDmyClk = 0x01;
  boot_header->flashCfg.cfg.fastReadDioCmd = 0xBB;
  boot_header->flashCfg.cfg.frDioDmyClk = 0x00;
  boot_header->flashCfg.cfg.fastReadQoCmd = 0x6B;
  boot_header->flashCfg.cfg.frQoDmyClk = 0x01;
  boot_header->flashCfg.cfg.fastReadQioCmd = 0xEB;
  boot_header->flashCfg.cfg.frQioDmyClk = 0x02;
  boot_header->flashCfg.cfg.qpiFastReadQioCmd = 0xEB;
  boot_header->flashCfg.cfg.qpiFrQioDmyClk = 0x02;
  boot_header->flashCfg.cfg.qpiPageProgramCmd = 0x02;
  boot_header->flashCfg.cfg.writeVregEnableCmd = 0x50;
  boot_header->flashCfg.cfg.wrEnableIndex = 0x00;
  boot_header->flashCfg.cfg.qeIndex = 0x01;
  boot_header->flashCfg.cfg.busyIndex = 0x00;
  boot_header->flashCfg.cfg.wrEnableBit = 0x01;
  boot_header->flashCfg.cfg.qeBit = 0x01;
  boot_header->flashCfg.cfg.busyBit = 0x00;
  boot_header->flashCfg.cfg.wrEnableWriteRegLen = 0x02;
  boot_header->flashCfg.cfg.wrEnableReadRegLen = 0x01;
  boot_header->flashCfg.cfg.qeWriteRegLen = 0x02;
  boot_header->flashCfg.cfg.qeReadRegLen = 0x01;
  boot_header->flashCfg.cfg.releasePowerDown = 0xAB;
  boot_header->flashCfg.cfg.busyReadRegLen = 0x01;
  boot_header->flashCfg.cfg.readRegCmd[0] = 0x05;
  boot_header->flashCfg.cfg.readRegCmd[1] = 0x00;
  boot_header->flashCfg.cfg.readRegCmd[2] = 0x00;
  boot_header->flashCfg.cfg.readRegCmd[3] = 0x00;
  boot_header->flashCfg.cfg.writeRegCmd[0] = 0x01;
  boot_header->flashCfg.cfg.writeRegCmd[1] = 0x00;
  boot_header->flashCfg.cfg.writeRegCmd[2] = 0x00;
  boot_header->flashCfg.cfg.writeRegCmd[3] = 0x00;
  boot_header->flashCfg.cfg.enterQpi = 0x38;
  boot_header->flashCfg.cfg.exitQpi = 0xFF;
  boot_header->flashCfg.cfg.cReadMode = 0x00;
  boot_header->flashCfg.cfg.cRExit = 0xFF;
  boot_header->flashCfg.cfg.burstWrapCmd = 0x77;
  boot_header->flashCfg.cfg.burstWrapCmdDmyClk = 0x03;
  boot_header->flashCfg.cfg.burstWrapDataMode = 0x02;
  boot_header->flashCfg.cfg.burstWrapData = 0x40;
  boot_header->flashCfg.cfg.deBurstWrapCmd = 0x77;
  boot_header->flashCfg.cfg.deBurstWrapCmdDmyClk = 0x03;
  boot_header->flashCfg.cfg.deBurstWrapDataMode = 0x02;
  boot_header->flashCfg.cfg.deBurstWrapData = 0xF0;
  boot_header->flashCfg.cfg.timeEsector = 0x12C;
  boot_header->flashCfg.cfg.timeE32k = 0x4B0;
  boot_header->flashCfg.cfg.timeE64k = 0x4B0;
  boot_header->flashCfg.cfg.timePagePgm = 0x05;
  boot_header->flashCfg.cfg.timeCe = 0xFFFF;
  boot_header->flashCfg.cfg.pdDelay = 0x14;
  boot_header->flashCfg.cfg.qeData = 0x00;
  boot_header->flashCfg.crc32 = 0xE43C762A;
  boot_header->clkCfg.cfg.xtal_type = 0x01;
  boot_header->clkCfg.cfg.pll_clk = 0x04;
  boot_header->clkCfg.cfg.hclk_div = 0x00;
  boot_header->clkCfg.cfg.bclk_div = 0x01;
  boot_header->clkCfg.cfg.flash_clk_type = 0x03;
  boot_header->clkCfg.cfg.flash_clk_div = 0x00;
  boot_header->clkCfg.crc32 = 0x72127DBA;
  boot_header->bootcfg.bval.sign = 0x00;
  boot_header->bootcfg.bval.encrypt_type = 0x00;
  boot_header->bootcfg.bval.key_sel = 0x00;
  boot_header->bootcfg.bval.rsvd6_7 = 0x00;
  boot_header->bootcfg.bval.no_segment = 0x01;
  boot_header->bootcfg.bval.cache_enable = 0x01;
  boot_header->bootcfg.bval.notload_in_bootrom = 0x00;
  boot_header->bootcfg.bval.aes_region_lock = 0x00;
  boot_header->bootcfg.bval.cache_way_disable = 0x00;
  boot_header->bootcfg.bval.crc_ignore = 0x01;
  boot_header->bootcfg.bval.hash_ignore = 0x01;
  boot_header->bootcfg.bval.halt_ap = 0x00;
  boot_header->bootcfg.bval.rsvd19_31 = 0x00;
  boot_header->segment_info.segment_cnt = 0xCDA8;
  boot_header->bootentry = 0x00;
  boot_header->flashoffset = 0x2000;
  boot_header->hash[0x00] = 0xEF;
  boot_header->hash[0x01] = 0xBE;
  boot_header->hash[0x02] = 0xAD;
  boot_header->hash[0x03] = 0xDE;
  boot_header->hash[0x04] = 0x00;
  boot_header->hash[0x05] = 0x00;
  boot_header->hash[0x06] = 0x00;
  boot_header->hash[0x07] = 0x00;
  boot_header->hash[0x08] = 0x00;
  boot_header->hash[0x09] = 0x00;
  boot_header->hash[0x0a] = 0x00;
  boot_header->hash[0x0b] = 0x00;
  boot_header->hash[0x0c] = 0x00;
  boot_header->hash[0x0d] = 0x00;
  boot_header->hash[0x0e] = 0x00;
  boot_header->hash[0x0f] = 0x00;
  boot_header->hash[0x10] = 0x00;
  boot_header->hash[0x11] = 0x00;
  boot_header->hash[0x12] = 0x00;
  boot_header->hash[0x13] = 0x00;
  boot_header->hash[0x14] = 0x00;
  boot_header->hash[0x15] = 0x00;
  boot_header->hash[0x16] = 0x00;
  boot_header->hash[0x17] = 0x00;
  boot_header->hash[0x18] = 0x00;
  boot_header->hash[0x19] = 0x00;
  boot_header->hash[0x1a] = 0x00;
  boot_header->hash[0x1b] = 0x00;
  boot_header->hash[0x1c] = 0x00;
  boot_header->hash[0x1d] = 0x00;
  boot_header->hash[0x1e] = 0x00;
  boot_header->hash[0x1f] = 0x00;
  boot_header->rsv1 = 0x1000;
  boot_header->rsv2 = 0x2000;
  boot_header->crc32 = 0xDEADBEEF;
}

blisp_return_t blisp_flash_firmware(void) {
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

  if (ret != 0) {
    return ret;
  }

  if (blisp_common_prepare_flash(&device) != 0) {
    // TODO: Error handling
    goto exit1;
  }
  parsed_firmware_file_t parsed_file;
  memset(&parsed_file, 0, sizeof(parsed_file));
  int parsed_result =
      parse_firmware_file(binary_to_write->filename[0], &parsed_file);

  // If we are injecting a bootloader section, make it, erase flash, and flash
  // it. Then when we do firmware later on; it will be located afterwards
  // the header filles up to a flash erase boundry so this stack should be safe
  // __should__

  if (parsed_file.needs_boot_struct) {
    // Create a default boot header section in ram to be written out
    struct bfl_boot_header boot_header;
    fill_up_boot_header(&boot_header);
    printf("Erasing flash to flash boot header\n");
    ret = blisp_device_flash_erase(&device, 0x0000,
                                   sizeof(struct bfl_boot_header));

    if (ret != BLISP_OK) {
      fprintf(stderr, "Failed to erase flash.\n");
      goto exit2;
    }
    // Now burn the header

    printf("Flashing boot header...\n");
    ret = blisp_device_flash_write(&device, 0x0000, (uint8_t*)&boot_header,
                                   sizeof(struct bfl_boot_header));
    if (ret != BLISP_OK) {
      fprintf(stderr, "Failed to write boot header.\n");
      goto exit2;
    }
    // Move the firmware to-be-flashed beyond the boot header area
    parsed_file.payload_address += 0x2000;
  }
  // Now that optional boot header is done, we clear out the flash for the new
  // firmware; and flash it in.

  printf("Erasing flash for firmware, this might take a while...\n");
  ret = blisp_device_flash_erase(
      &device, parsed_file.payload_address,
      parsed_file.payload_address + parsed_file.payload_length);

  if (ret != BLISP_OK) {
    fprintf(stderr,
            "Failed to erase flash. Tried to erase from 0x%08X to 0x%08X\n",
            parsed_file.payload_address,
            parsed_file.payload_address + parsed_file.payload_length + 1);
    goto exit2;
  }

  printf("Flashing the firmware %d bytes @ 0x%08X...\n",
         parsed_file.payload_length, parsed_file.payload_address);
  struct blisp_easy_transport data_transport =
      blisp_easy_transport_new_from_memory(parsed_file.payload,
                                           parsed_file.payload_length);

  ret = blisp_easy_flash_write(
      &device, &data_transport, parsed_file.payload_address,
      parsed_file.payload_length, blisp_common_progress_callback);

  if (ret < BLISP_OK) {
    fprintf(stderr, "Failed to write app to flash.\n");
    goto exit2;
  }

  printf("Checking program...\n");
  ret = blisp_device_program_check(&device);
  if (ret != BLISP_OK) {
    fprintf(stderr, "Failed to check program.\n");
    goto exit2;
  }
  printf("Program OK!\n");

  if (reset->count > 0) {
    blisp_device_reset(&device);
    printf("Resetting the chip.\n");
    // TODO: It seems that GPIO peripheral is not reset after resetting the chip
  }

  printf("Flash complete!\n");

exit2:
  if (parsed_file.payload != NULL)
    free(parsed_file.payload);
exit1:
  blisp_device_close(&device);
}

blisp_return_t cmd_write_args_init(void) {
  size_t index = 0;

  cmd_write_argtable[index++] = cmd =
      arg_rex1(NULL, NULL, "write", NULL, REG_ICASE, NULL);
  cmd_write_argtable[index++] = chip_type =
      arg_str1("c", "chip", "<chip_type>", "Chip Type");
  cmd_write_argtable[index++] = port_name =
      arg_str0("p", "port", "<port_name>",
               "Name/Path to the Serial Port (empty for search)");
  cmd_write_argtable[index++] = baudrate =
      arg_int0("b", "baudrate", "<baud rate>",
               "Serial baud rate (default: " XSTR(DEFAULT_BAUDRATE) ")");
  cmd_write_argtable[index++] = reset =
      arg_lit0(NULL, "reset", "Reset chip after write");
  cmd_write_argtable[index++] = binary_to_write =
      arg_file1(NULL, NULL, "<input>", "Binary to write");
  cmd_write_argtable[index++] = end = arg_end(10);

  if (arg_nullcheck(cmd_write_argtable) != 0) {
    fprintf(stderr, "insufficient memory\n");
    return BLISP_ERR_OUT_OF_MEMORY;
  }
  return BLISP_OK;
}

void cmd_write_args_print_glossary(void) {
  fputs("Usage: blisp", stdout);
  arg_print_syntax(stdout, cmd_write_argtable, "\n");
  puts("Writes firmware to SPI Flash");
  arg_print_glossary(stdout, cmd_write_argtable, "  %-25s %s\n");
}

blisp_return_t cmd_write_parse_exec(int argc, char** argv) {
  int errors = arg_parse(argc, argv, cmd_write_argtable);
  if (errors == 0) {
    return blisp_flash_firmware();  // TODO: Error code?

  } else if (cmd->count == 1) {
    cmd_write_args_print_glossary();
    return BLISP_OK;
  }
  return BLISP_ERR_INVALID_COMMAND;
}

void cmd_write_args_print_syntax(void) {
  arg_print_syntax(stdout, cmd_write_argtable, "\n");
}

void cmd_write_free(void) {
  arg_freetable(cmd_write_argtable,
                sizeof(cmd_write_argtable) / sizeof(cmd_write_argtable[0]));
}

struct cmd cmd_write = {"write", cmd_write_args_init, cmd_write_parse_exec,
                        cmd_write_args_print_syntax, cmd_write_free};
