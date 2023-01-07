// SPDX-License-Identifier: MIT
#include <blisp.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include "../cmd.h"
#include "argtable3.h"
#include "blisp_struct.h"
#include "../util.h"

#ifdef __linux__
#include <linux/limits.h>
#endif

#define REG_EXTENDED 1
#define REG_ICASE (REG_EXTENDED << 1)

static struct arg_rex* cmd;
static struct arg_file* binary_to_write;
static struct arg_str *port_name, *chip_type;
static struct arg_lit* reset;
static struct arg_end* end;
static void* cmd_write_argtable[6];

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

void blisp_flash_firmware() {
  FILE* eflash_loader_file = NULL;

  if (chip_type->count == 0) {
    fprintf(stderr, "Chip type is invalid.\n");
    return;
  }

  struct blisp_chip* chip = NULL;

  if (strcmp(chip_type->sval[0], "bl70x") == 0) {
    chip = &blisp_chip_bl70x;
  } else if (strcmp(chip_type->sval[0], "bl60x") == 0) {
    chip = &blisp_chip_bl60x;
  } else {
    fprintf(stderr, "Chip type is invalid.\n");
    return;
  }

  struct blisp_device device;
  int32_t ret;
  ret = blisp_device_init(&device, chip);
  if (ret != BLISP_OK) {
    fprintf(stderr, "Failed to init device.\n");
    return;
  }
  ret = blisp_device_open(&device,
                          port_name->count == 1 ? port_name->sval[0] : NULL);
  if (ret != BLISP_OK) {
    fprintf(stderr, "Failed to open device.\n");
    return;
  }
  printf("Sending a handshake...");
  ret = blisp_device_handshake(&device, false);
  if (ret != BLISP_OK) {
    fprintf(stderr, "\nFailed to handshake with device.\n");
    goto exit1;
  }
  printf(" OK\nGetting chip info...");
  struct blisp_boot_info boot_info;
  ret = blisp_device_get_boot_info(&device, &boot_info);
  if (ret != BLISP_OK) {
    fprintf(stderr, "\nFailed to get boot info.\n");
    goto exit1;
  }

  if (boot_info.boot_rom_version[0] == 255 &&
      boot_info.boot_rom_version[1] == 255 &&
      boot_info.boot_rom_version[2] == 255 &&
      boot_info.boot_rom_version[3] == 255) {
    printf(" OK\nDevice already in eflash_loader.\n");
    goto eflash_loader;
  }

  printf(
      " BootROM version %d.%d.%d.%d, ChipID: "
      "%02X%02X%02X%02X%02X%02X%02X%02X\n",
      boot_info.boot_rom_version[0], boot_info.boot_rom_version[1],
      boot_info.boot_rom_version[2], boot_info.boot_rom_version[3],
      boot_info.chip_id[0], boot_info.chip_id[1], boot_info.chip_id[2],
      boot_info.chip_id[3], boot_info.chip_id[4], boot_info.chip_id[5],
      boot_info.chip_id[6], boot_info.chip_id[7]);

  char exe_path[PATH_MAX];
  char eflash_loader_path[PATH_MAX];
  if (util_get_binary_folder(exe_path, PATH_MAX) <= 0) {
    fprintf(stderr,
            "Failed to find executable path to search for the "
            "eflash loader\n");
    goto exit1;
  }
  snprintf(eflash_loader_path, PATH_MAX, "%s/data/%s/eflash_loader_%s.bin",
           exe_path, device.chip->type_str,
           device.chip->default_xtal);
  printf("Loading the eflash loader file from disk\n");
  eflash_loader_file = fopen(eflash_loader_path, "rb");  // TODO: Error handling
  if (eflash_loader_file == NULL) {
    fprintf(stderr,
            "Could not open the eflash loader file from disk.\n"
            "Does \"%s\" exist?\n",
            eflash_loader_path);
    goto exit1;
  }
  uint8_t
      eflash_loader_header[176];  // TODO: Remap it to the boot header struct
  fread(eflash_loader_header, 176, 1,
        eflash_loader_file);  // TODO: Error handling

  printf("Loading eflash_loader...\n");
  ret = blisp_device_load_boot_header(&device, eflash_loader_header);
  if (ret != BLISP_OK) {
    fprintf(stderr, "Failed to load boot header.\n");
    goto exit1;
  }

  {
    uint32_t sent_data = 0;
    uint32_t buffer_size = 0;
    uint8_t buffer[4092];

    // TODO: Real checking of segments count
    for (uint8_t seg_index = 0; seg_index < 1; seg_index++) {
      struct blisp_segment_header segment_header = {0};
      fread(&segment_header, 16, 1,
            eflash_loader_file);  // TODO: Error handling

      ret = blisp_device_load_segment_header(&device, &segment_header);
      if (ret != 0) {
        fprintf(stderr, "Failed to load segment header.\n");
        goto exit1;
      }
      printf("Flashing %d. segment\n", seg_index + 1);
      printf("0b / %" PRIu32 "b (0.00%%)\n", segment_header.length);

      while (sent_data < segment_header.length) {
        buffer_size = segment_header.length - sent_data;
        if (buffer_size > 4092) {
          buffer_size = 4092;
        }
        fread(buffer, buffer_size, 1, eflash_loader_file);
        ret = blisp_device_load_segment_data(
            &device, buffer, buffer_size);  // TODO: Error handling
        if (ret < BLISP_OK) {
          fprintf(stderr, "Failed to load segment data. (ret %d)\n", ret);
          goto exit1;
        }
        sent_data += buffer_size;
        printf("%" PRIu32 "b / %" PRIu32 "b (%.2f%%)\n", sent_data,
               segment_header.length,
               (((float)sent_data / (float)segment_header.length) * 100.0f));
      }
    }
  }

  ret = blisp_device_check_image(&device);
  if (ret != BLISP_OK) {
    fprintf(stderr, "Failed to check image.\n");
    goto exit1;
  }

  ret = blisp_device_run_image(&device);
  if (ret != BLISP_OK) {
    fprintf(stderr, "Failed to run image.\n");
    goto exit1;
  }

  printf("Sending a handshake...");
  ret = blisp_device_handshake(&device, true);
  if (ret != BLISP_OK) {
    fprintf(stderr, "\nFailed to handshake with device.\n");
    goto exit1;
  }
  printf(" OK\n");

eflash_loader:;
  FILE* firmware_file = fopen(binary_to_write->filename[0], "rb");
  if (firmware_file == NULL) {
    fprintf(stderr, "Failed to open firmware file \"%s\".\n",
            binary_to_write->filename[0]);
    goto exit1;
  }
  fseek(firmware_file, 0, SEEK_END);
  int64_t firmware_file_size = ftell(firmware_file);
  rewind(firmware_file);

  struct bfl_boot_header boot_header;
  fill_up_boot_header(&boot_header);

  const uint32_t firmware_base_address = 0x2000;
  printf("Erasing flash, this might take a while...");
  ret =
      blisp_device_flash_erase(&device, firmware_base_address,
                               firmware_base_address + firmware_file_size + 1);
  if (ret != BLISP_OK) {
    fprintf(stderr, "\nFailed to erase flash.\n");
    goto exit2;
  }
  ret =
      blisp_device_flash_erase(&device, 0x0000, sizeof(struct bfl_boot_header));
  if (ret != BLISP_OK) {
    fprintf(stderr, "\nFailed to erase flash.\n");
    goto exit2;
  }

  printf(" OK!\nFlashing boot header...");
  ret = blisp_device_flash_write(&device, 0x0000, (uint8_t*)&boot_header,
                                 sizeof(struct bfl_boot_header));
  if (ret != BLISP_OK) {
    fprintf(stderr, "\nFailed to write boot header.\n");
    goto exit2;
  }
  printf(" OK!\nFlashing the firmware...\n");
  {
    uint32_t sent_data = 0;
    uint32_t buffer_size = 0;
    uint8_t buffer[8184];
    printf("0b / %ldb (0.00%%)\n", firmware_file_size);

    while (sent_data < firmware_file_size) {
      buffer_size = firmware_file_size - sent_data;
      if (buffer_size > 2052) {
        buffer_size = 2052;
      }
      fread(buffer, buffer_size, 1, firmware_file);
      ret = blisp_device_flash_write(&device, firmware_base_address + sent_data,
                                     buffer,
                                     buffer_size);  // TODO: Error handling
      if (ret < BLISP_OK) {
        fprintf(stderr, "Failed to write firmware! (ret: %d)\n", ret);
        goto exit2;
      }
      sent_data += buffer_size;
      printf("%" PRIu32 "b / %ldb (%.2f%%)\n", sent_data, firmware_file_size,
             (((float)sent_data / (float)firmware_file_size) * 100.0f));
    }
  }

  printf("Checking program...");
  ret = blisp_device_program_check(&device);
  if (ret != BLISP_OK) {
    fprintf(stderr, "\nFailed to check program.\n");
    goto exit2;
  }
  printf("OK\n");

  if (reset->count > 0) {
    blisp_device_reset(&device);
    printf("Resetting the chip.\n");
    // TODO: It seems that GPIO peripheral is not reset after resetting the chip
  }

  printf("Flash complete!\n");

exit2:
  if (firmware_file != NULL)
    fclose(firmware_file);
exit1:
  if (eflash_loader_file != NULL)
    fclose(eflash_loader_file);
  blisp_device_close(&device);
}

int8_t cmd_write_args_init() {
  cmd_write_argtable[0] = cmd =
      arg_rex1(NULL, NULL, "write", NULL, REG_ICASE, NULL);
  cmd_write_argtable[1] = chip_type =
      arg_str1("c", "chip", "<chip_type>", "Chip Type (bl70x)");
  cmd_write_argtable[2] = port_name =
      arg_str0("p", "port", "<port_name>",
               "Name/Path to the Serial Port (empty for search)");
  cmd_write_argtable[3] = reset =
      arg_lit0(NULL, "reset", "Reset chip after write");
  cmd_write_argtable[4] = binary_to_write =
      arg_file1(NULL, NULL, "<input>", "Binary to write");
  cmd_write_argtable[5] = end = arg_end(10);

  if (arg_nullcheck(cmd_write_argtable) != 0) {
    fprintf(stderr, "insufficient memory\n");
    return -1;
  }
  return 0;
}

void cmd_write_args_print_glossary() {
  fputs("Usage: blisp", stdout);
  arg_print_syntax(stdout, cmd_write_argtable, "\n");
  puts("Writes firmware to SPI Flash");
  arg_print_glossary(stdout, cmd_write_argtable, "  %-25s %s\n");
}

uint8_t cmd_write_parse_exec(int argc, char** argv) {
  int errors = arg_parse(argc, argv, cmd_write_argtable);
  if (errors == 0) {
    blisp_flash_firmware();  // TODO: Error code?
    return 1;
  } else if (cmd->count == 1) {
    cmd_write_args_print_glossary();
    return 1;
  }
  return 0;
}

void cmd_write_args_print_syntax() {
  arg_print_syntax(stdout, cmd_write_argtable, "\n");
}

void cmd_write_free() {
  arg_freetable(cmd_write_argtable,
                sizeof(cmd_write_argtable) / sizeof(cmd_write_argtable[0]));
}

struct cmd cmd_write = {"write", cmd_write_args_init, cmd_write_parse_exec,
                        cmd_write_args_print_syntax, cmd_write_free};
