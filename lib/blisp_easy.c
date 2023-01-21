// SPDX-License-Identifier: MIT
#include "blisp_easy.h"
#include "blisp_struct.h"
#include "blisp_util.h"

#include <inttypes.h>
#include <string.h>

static int64_t blisp_easy_transport_read(struct blisp_easy_transport* transport,
                                         void* buffer,
                                         uint32_t size) {
  if (transport->type == 0) {
    // TODO: Implement reading more than available
    memcpy(buffer, (uint8_t*)transport->data.memory.data_location + transport->data.memory.current_position, size);
    transport->data.memory.current_position += size;
    return size;
  } else {
    return fread(buffer, size, 1, transport->data.file_handle);
  }
}

static int64_t blisp_easy_transport_size(struct blisp_easy_transport* transport) {
  if (transport->type == 0) {
    return transport->data.memory.data_size;
  } else {
    // TODO: Implement
  }
}

static void blisp_easy_report_progress(blisp_easy_progress_callback callback,
                                       uint32_t current_value,
                                       uint32_t max_value) {
  if (callback != NULL) {
    callback(current_value, max_value);
  }
}

struct blisp_easy_transport blisp_easy_transport_new_from_file(FILE* file) {
  struct blisp_easy_transport transport = {.type = 1, .data.file_handle = file};
  return transport;
}

struct blisp_easy_transport blisp_easy_transport_new_from_memory(
    void* data_location,
    uint32_t data_size) {
  struct blisp_easy_transport transport = {
      .type = 0,
      .data.memory.data_location = data_location,
      .data.memory.data_size = data_size,
      .data.memory.current_position = 0};
  return transport;
}

int32_t blisp_easy_load_segment_data(
    struct blisp_device* device,
    uint32_t segment_size,
    struct blisp_easy_transport* segment_transport,
    blisp_easy_progress_callback progress_callback) {
  int32_t ret;
#ifdef __APPLE__
  const uint16_t buffer_max_size = 252 * 16;
#else
  const uint16_t buffer_max_size = 4092;
#endif


  uint32_t sent_data = 0;
  uint32_t buffer_size = 0;
#ifdef _WIN32
  uint8_t buffer[4092];
#else
  uint8_t buffer[buffer_max_size];
#endif

  blisp_easy_report_progress(progress_callback, 0, segment_size);

  while (sent_data < segment_size) {
    buffer_size = segment_size - sent_data;
    if (buffer_size > buffer_max_size) {
      buffer_size = buffer_max_size;
    }
    blisp_easy_transport_read(segment_transport, buffer,
                              buffer_size);  // TODO: Error Handling
    ret = blisp_device_load_segment_data(device, buffer, buffer_size);
    if (ret < BLISP_OK) {
      // TODO: Error logging fprintf(stderr, "Failed to load segment data. (ret
      // %d)\n", ret);
      return ret;
    }
    sent_data += buffer_size;
    blisp_easy_report_progress(progress_callback, sent_data, segment_size);
  }
  return 0;
}

int32_t blisp_easy_load_ram_image(
    struct blisp_device* device,
    struct blisp_easy_transport* image_transport,
    blisp_easy_progress_callback progress_callback) {
  int32_t ret;

  struct bfl_boot_header image_boot_header;
  // TODO: Error handling
  blisp_easy_transport_read(image_transport, &image_boot_header, 176);

  ret = blisp_device_load_boot_header(device, (uint8_t*)&image_boot_header);
  if (ret != BLISP_OK) {
    // TODO: Error printing: fprintf(stderr, "Failed to load boot header.\n");
    return ret;
  }

  {
    for (uint8_t seg_index = 0;
         seg_index < image_boot_header.segment_info.segment_cnt; seg_index++) {
      struct blisp_segment_header segment_header = {0};
      blisp_easy_transport_read(image_transport, &segment_header,
                                16);  // TODO: Error handling

      ret = blisp_device_load_segment_header(device, &segment_header);
      if (ret != 0) {
        // TODO: Error printing: fprintf(stderr, "Failed to load segment
        // header.");
        return ret;
      }
      // TODO: Info printing: printf("Flashing %d. segment\n", seg_index + 1);

      ret = blisp_easy_load_segment_data(device, segment_header.length,
                                         image_transport, progress_callback);
      if (ret != 0) {
        return ret;
      }
    }
  }

  ret = blisp_device_check_image(device);
  if (ret != BLISP_OK) {
    // TODO: Error printing: fprintf(stderr, "Failed to check image.\n");
    return BLISP_EASY_ERR_CHECK_IMAGE_FAILED;
  }

  return BLISP_OK;
}

int32_t blisp_easy_load_ram_app(struct blisp_device* device,
                                struct blisp_easy_transport* app_transport,
                                blisp_easy_progress_callback progress_callback)
{
  int32_t ret;
  // TODO: Rework
  // region boot header fill
  struct bfl_boot_header boot_header;
  memcpy(boot_header.magiccode, "BFNP", 4);
  memcpy(boot_header.flashCfg.magiccode, "FCFG", 4);
  boot_header.revison = 0x01;
  boot_header.flashCfg.cfg.ioMode = 0x04;
  boot_header.flashCfg.cfg.cReadSupport = 0x01;
  boot_header.flashCfg.cfg.clkDelay = 0x01;
  boot_header.flashCfg.cfg.clkInvert = 0x01;
  boot_header.flashCfg.cfg.resetEnCmd = 0x66;
  boot_header.flashCfg.cfg.resetCmd = 0x99;
  boot_header.flashCfg.cfg.resetCreadCmd = 0xFF;
  boot_header.flashCfg.cfg.resetCreadCmdSize = 0x03;
  boot_header.flashCfg.cfg.jedecIdCmd = 0x9F;
  boot_header.flashCfg.cfg.jedecIdCmdDmyClk = 0x00;
  boot_header.flashCfg.cfg.qpiJedecIdCmd = 0x9F;
  boot_header.flashCfg.cfg.qpiJedecIdCmdDmyClk = 0x00;
  boot_header.flashCfg.cfg.sectorSize = 0x04;
  boot_header.flashCfg.cfg.mid = 0xEF;
  boot_header.flashCfg.cfg.pageSize = 0x100;
  boot_header.flashCfg.cfg.chipEraseCmd = 0xC7;
  boot_header.flashCfg.cfg.sectorEraseCmd = 0x20;
  boot_header.flashCfg.cfg.blk32EraseCmd = 0x52;
  boot_header.flashCfg.cfg.blk64EraseCmd = 0xD8;
  boot_header.flashCfg.cfg.writeEnableCmd = 0x06;
  boot_header.flashCfg.cfg.pageProgramCmd = 0x02;
  boot_header.flashCfg.cfg.qpageProgramCmd = 0x32;
  boot_header.flashCfg.cfg.qppAddrMode = 0x00;
  boot_header.flashCfg.cfg.fastReadCmd = 0x0B;
  boot_header.flashCfg.cfg.frDmyClk = 0x01;
  boot_header.flashCfg.cfg.qpiFastReadCmd = 0x0B;
  boot_header.flashCfg.cfg.qpiFrDmyClk = 0x01;
  boot_header.flashCfg.cfg.fastReadDoCmd = 0x3B;
  boot_header.flashCfg.cfg.frDoDmyClk = 0x01;
  boot_header.flashCfg.cfg.fastReadDioCmd = 0xBB;
  boot_header.flashCfg.cfg.frDioDmyClk = 0x00;
  boot_header.flashCfg.cfg.fastReadQoCmd = 0x6B;
  boot_header.flashCfg.cfg.frQoDmyClk = 0x01;
  boot_header.flashCfg.cfg.fastReadQioCmd = 0xEB;
  boot_header.flashCfg.cfg.frQioDmyClk = 0x02;
  boot_header.flashCfg.cfg.qpiFastReadQioCmd = 0xEB;
  boot_header.flashCfg.cfg.qpiFrQioDmyClk = 0x02;
  boot_header.flashCfg.cfg.qpiPageProgramCmd = 0x02;
  boot_header.flashCfg.cfg.writeVregEnableCmd = 0x50;
  boot_header.flashCfg.cfg.wrEnableIndex = 0x00;
  boot_header.flashCfg.cfg.qeIndex = 0x01;
  boot_header.flashCfg.cfg.busyIndex = 0x00;
  boot_header.flashCfg.cfg.wrEnableBit = 0x01;
  boot_header.flashCfg.cfg.qeBit = 0x01;
  boot_header.flashCfg.cfg.busyBit = 0x00;
  boot_header.flashCfg.cfg.wrEnableWriteRegLen = 0x02;
  boot_header.flashCfg.cfg.wrEnableReadRegLen = 0x01;
  boot_header.flashCfg.cfg.qeWriteRegLen = 0x01;
  boot_header.flashCfg.cfg.qeReadRegLen = 0x01;
  boot_header.flashCfg.cfg.releasePowerDown = 0xAB;
  boot_header.flashCfg.cfg.busyReadRegLen = 0x01;
  boot_header.flashCfg.cfg.readRegCmd[0] = 0x05;
  boot_header.flashCfg.cfg.readRegCmd[1] = 0x35;
  boot_header.flashCfg.cfg.readRegCmd[2] = 0x00;
  boot_header.flashCfg.cfg.readRegCmd[3] = 0x00;
  boot_header.flashCfg.cfg.writeRegCmd[0] = 0x01;
  boot_header.flashCfg.cfg.writeRegCmd[1] = 0x31;
  boot_header.flashCfg.cfg.writeRegCmd[2] = 0x00;
  boot_header.flashCfg.cfg.writeRegCmd[3] = 0x00;
  boot_header.flashCfg.cfg.enterQpi = 0x38;
  boot_header.flashCfg.cfg.exitQpi = 0xFF;
  boot_header.flashCfg.cfg.cReadMode = 0x20;
  boot_header.flashCfg.cfg.cRExit = 0xFF;
  boot_header.flashCfg.cfg.burstWrapCmd = 0x77;
  boot_header.flashCfg.cfg.burstWrapCmdDmyClk = 0x03;
  boot_header.flashCfg.cfg.burstWrapDataMode = 0x02;
  boot_header.flashCfg.cfg.burstWrapData = 0x40;
  boot_header.flashCfg.cfg.deBurstWrapCmd = 0x77;
  boot_header.flashCfg.cfg.deBurstWrapCmdDmyClk = 0x03;
  boot_header.flashCfg.cfg.deBurstWrapDataMode = 0x02;
  boot_header.flashCfg.cfg.deBurstWrapData = 0xF0;
  boot_header.flashCfg.cfg.timeEsector = 0x12C;
  boot_header.flashCfg.cfg.timeE32k = 0x4B0;
  boot_header.flashCfg.cfg.timeE64k = 0x4B0;
  boot_header.flashCfg.cfg.timePagePgm = 0x05;
  boot_header.flashCfg.cfg.timeCe = 0xD40;
  boot_header.flashCfg.cfg.pdDelay = 0x03;
  boot_header.flashCfg.cfg.qeData = 0x00;
  boot_header.flashCfg.crc32 = 0xC4BDD748;
  boot_header.clkCfg.cfg.xtal_type = 0x04;
  boot_header.clkCfg.cfg.pll_clk = 0x04;
  boot_header.clkCfg.cfg.hclk_div = 0x00;
  boot_header.clkCfg.cfg.bclk_div = 0x01;
  boot_header.clkCfg.cfg.flash_clk_type = 0x02;
  boot_header.clkCfg.cfg.flash_clk_div = 0x00;
  boot_header.clkCfg.crc32 = 0x824E14BB;
  boot_header.bootcfg.bval.sign = 0x00;
  boot_header.bootcfg.bval.encrypt_type = 0x00;
  boot_header.bootcfg.bval.key_sel = 0x00;
  boot_header.bootcfg.bval.rsvd6_7 = 0x00;
  boot_header.bootcfg.bval.no_segment = 0x01;
  boot_header.bootcfg.bval.cache_enable = 0x01;
  boot_header.bootcfg.bval.notload_in_bootrom = 0x00;
  boot_header.bootcfg.bval.aes_region_lock = 0x00;
  boot_header.bootcfg.bval.cache_way_disable = 0x00;
  boot_header.bootcfg.bval.crc_ignore = 0x01;
  boot_header.bootcfg.bval.hash_ignore = 0x01;
  boot_header.bootcfg.bval.halt_ap = 0x00;
  boot_header.bootcfg.bval.rsvd19_31 = 0x00;
  boot_header.segment_info.segment_cnt = 0x01;
  boot_header.bootentry = 0x00;
  boot_header.flashoffset = device->chip->tcm_address;
  boot_header.hash[0x00] = 0xEF;
  boot_header.hash[0x01] = 0xBE;
  boot_header.hash[0x02] = 0xAD;
  boot_header.hash[0x03] = 0xDE;
  boot_header.hash[0x04] = 0x00;
  boot_header.hash[0x05] = 0x00;
  boot_header.hash[0x06] = 0x00;
  boot_header.hash[0x07] = 0x00;
  boot_header.hash[0x08] = 0x00;
  boot_header.hash[0x09] = 0x00;
  boot_header.hash[0x0a] = 0x00;
  boot_header.hash[0x0b] = 0x00;
  boot_header.hash[0x0c] = 0x00;
  boot_header.hash[0x0d] = 0x00;
  boot_header.hash[0x0e] = 0x00;
  boot_header.hash[0x0f] = 0x00;
  boot_header.hash[0x10] = 0x00;
  boot_header.hash[0x11] = 0x00;
  boot_header.hash[0x12] = 0x00;
  boot_header.hash[0x13] = 0x00;
  boot_header.hash[0x14] = 0x00;
  boot_header.hash[0x15] = 0x00;
  boot_header.hash[0x16] = 0x00;
  boot_header.hash[0x17] = 0x00;
  boot_header.hash[0x18] = 0x00;
  boot_header.hash[0x19] = 0x00;
  boot_header.hash[0x1a] = 0x00;
  boot_header.hash[0x1b] = 0x00;
  boot_header.hash[0x1c] = 0x00;
  boot_header.hash[0x1d] = 0x00;
  boot_header.hash[0x1e] = 0x00;
  boot_header.hash[0x1f] = 0x00;
  boot_header.rsv1 = 0x00;
  boot_header.rsv2 = 0x00;
  boot_header.crc32 = 0xDEADBEEF;
  // endregion


  ret = blisp_device_load_boot_header(device, (uint8_t*)&boot_header);
  if (ret != BLISP_OK) {
    blisp_dlog("Failed to load boot header, ret: %d.", ret);
    return ret;
  }

  struct blisp_segment_header segment_header = {
      .dest_addr = device->chip->tcm_address,
      .length = blisp_easy_transport_size(app_transport),
      .reserved = 0,
      .crc32 = 0
  };
  segment_header.crc32 = crc32_calculate(&segment_header, 3 * sizeof(uint32_t)); // TODO: Make function

  ret = blisp_device_load_segment_header(device, &segment_header);
  if (ret != 0) {
    blisp_dlog("Failed to load segment header, ret: %d.", ret);
    return ret;
  }

  ret = blisp_easy_load_segment_data(device, blisp_easy_transport_size(app_transport),
                                     app_transport, progress_callback);
  if (ret != 0) {
    // TODO: Error printing
    return ret;
  }


  return BLISP_OK;
}

int32_t blisp_easy_flash_write(struct blisp_device* device,
                               struct blisp_easy_transport* data_transport,
                               uint32_t flash_location,
                               uint32_t data_size,
                               blisp_easy_progress_callback progress_callback) {
  int32_t ret;
#ifdef __APPLE__
  const uint16_t buffer_max_size = 372 * 1;
#else
  const uint16_t buffer_max_size = 2052;
#endif

  uint32_t sent_data = 0;
  uint32_t buffer_size = 0;
#ifdef _WIN32
  uint8_t buffer[2052];
#else
  uint8_t buffer[buffer_max_size];
#endif
  blisp_easy_report_progress(progress_callback, 0, data_size);

  while (sent_data < data_size) {
    buffer_size = data_size - sent_data;
    if (buffer_size > buffer_max_size) {
      buffer_size = buffer_max_size;
    }
    blisp_easy_transport_read(data_transport, buffer,
                              buffer_size);  // TODO: Error Handling
    ret = blisp_device_flash_write(device, flash_location + sent_data, buffer,
                                   buffer_size);
    if (ret < BLISP_OK) {
      // TODO: Error logigng:  fprintf(stderr, "Failed to write firmware! (ret:
      // %d)\n", ret);
      return ret;
    }
    sent_data += buffer_size;
    blisp_easy_report_progress(progress_callback, sent_data, data_size);
  }
  return BLISP_OK;
}