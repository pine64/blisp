// SPDX-License-Identifier: MIT
#include <blisp.h>
#include <blisp_util.h>
#include <libserialport.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __linux__
#include <linux/serial.h>
#include <sys/ioctl.h>
#endif

#include <blisp_struct.h>

#define DEBUG

static void drain(struct sp_port* port) {
#if defined(__APPLE__) || defined(__FreeBSD__)
  sp_drain(port);
#endif
}

blisp_return_t blisp_device_init(struct blisp_device* device,
                                 struct blisp_chip* chip) {
  device->chip = chip;
  device->is_usb = false;
  fill_crcs(&bl808_header);

  if (device->chip->type == BLISP_CHIP_BL808) {
    // TODO: For some reason the BL808 does not send pending ('PD') responses
    //       during long (i.e. erase) operations, so we must disable response
    //       timeouts. Further investigation is necessary.
    device->serial_timeout = 0;
  } else {
    device->serial_timeout = 1000;
  }

  return BLISP_OK;
}

blisp_return_t blisp_device_open(struct blisp_device* device,
                                 const char* port_name,
                                 uint32_t baudrate) {
  blisp_return_t ret;
  struct sp_port* serial_port = NULL;

  if (port_name != NULL) {
    ret = sp_get_port_by_name(port_name, &serial_port);
    if (ret != SP_OK) {
      blisp_dlog("Couldn't open device, err: %d", ret);
      return BLISP_ERR_CANT_OPEN_DEVICE;
    }
  } else {
    if (!device->chip->usb_isp_available) {
      return BLISP_ERR_NO_AUTO_FIND_AVAILABLE;
    }
    struct sp_port** port_list;
    ret = sp_list_ports(&port_list);
    if (ret != SP_OK) {
      blisp_dlog("Couldn't list ports, err: %d", ret);
      return BLISP_ERR_DEVICE_NOT_FOUND;
    }
    for (int i = 0; port_list[i] != NULL; i++) {
      struct sp_port* port = port_list[i];

      int vid, pid;
      sp_get_port_usb_vid_pid(port, &vid, &pid);
      if (vid == 0xFFFF && pid == 0xFFFF) {
        ret = sp_get_port_by_name(sp_get_port_name(port), &serial_port);
        if (ret != SP_OK) {
          blisp_dlog("Couldn't open device, err: %d", ret);
          return BLISP_ERR_CANT_OPEN_DEVICE;
        }
        break;
      }
    }
    sp_free_port_list(port_list);
    if (serial_port == NULL) {
      return BLISP_ERR_DEVICE_NOT_FOUND;
    }
  }

  ret = sp_open(serial_port, SP_MODE_READ_WRITE);
  if (ret != SP_OK) {
    blisp_dlog("SP open failed: %d", ret);
    return BLISP_ERR_CANT_OPEN_DEVICE;
  }
  // TODO: Handle errors in following functions, although, none of them *should*
  // fail
  sp_set_bits(serial_port, 8);
  sp_set_parity(serial_port, SP_PARITY_NONE);
  sp_set_stopbits(serial_port, 1);
  sp_set_flowcontrol(serial_port, SP_FLOWCONTROL_NONE);

  int vid, pid;
  sp_get_port_usb_vid_pid(serial_port, &vid, &pid);
  device->is_usb = pid == 0xFFFF;
  device->current_baud_rate = baudrate;

#if 0
    int fd;
    sp_get_port_handle(serial_port, &fd);
    struct serial_struct serial;
    ioctl(fd, TIOCGSERIAL, &serial);
//    serial.flags &= ~(ASYNC_LOW_LATENCY);
    serial.flags |= ASYNC_LOW_LATENCY;
    ioctl(fd, TIOCSSERIAL, &serial);
#endif
  ret = sp_set_baudrate(serial_port, device->current_baud_rate);
  if (ret != SP_OK) {
    blisp_dlog("Set baud rate failed: %d... Also hello MacOS user :)", ret);
    return BLISP_ERR_API_ERROR;
  }
  device->serial_port = serial_port;

  return BLISP_OK;
}

blisp_return_t blisp_send_command(struct blisp_device* device,
                                  uint8_t command,
                                  void* payload,
                                  uint16_t payload_size,
                                  bool add_checksum) {
  int ret;
  struct sp_port* serial_port = device->serial_port;

  device->tx_buffer[0] = command;
  device->tx_buffer[1] = 0;
  device->tx_buffer[2] = payload_size & 0xFF;
  device->tx_buffer[3] = (payload_size >> 8) & 0xFF;
  if (add_checksum) {
    uint32_t checksum = 0;
    checksum += device->tx_buffer[2] + device->tx_buffer[3];
    for (uint16_t i = 0; i < payload_size; i++) {
      checksum += *(uint8_t*)((uint8_t*)payload + i);
    }
    device->tx_buffer[1] = checksum & 0xFF;
  }
  if (payload_size != 0) {
    memcpy(&device->tx_buffer[4], payload, payload_size);
  }
  ret =
      sp_blocking_write(serial_port, device->tx_buffer, 4 + payload_size, 1000);
  if (ret != (4 + payload_size)) {
    blisp_dlog("Received error or not written all data: %d", ret);
    return BLISP_ERR_API_ERROR;
  }
  drain(serial_port);

  return BLISP_OK;
}

blisp_return_t blisp_receive_response(struct blisp_device* device,
                                      bool expect_payload) {
  // TODO: Check checksum
  int ret;

  struct sp_port* serial_port = device->serial_port;
  ret = sp_blocking_read(serial_port, &device->rx_buffer[0], 2, device->serial_timeout);
  if (ret < 2) {
    blisp_dlog("Failed to receive response, ret: %d", ret);
    return BLISP_ERR_NO_RESPONSE;
  } else if (device->rx_buffer[0] == 'O' && device->rx_buffer[1] == 'K') {
    if (expect_payload) {
      sp_blocking_read(serial_port, &device->rx_buffer[2], 2,
                       100);  // TODO: Check if really we received the data.
      uint16_t data_length =
          (device->rx_buffer[3] << 8) | (device->rx_buffer[2]);
      sp_blocking_read(serial_port, &device->rx_buffer[0], data_length, 100);
      return data_length;
    }
    return 0;
  } else if (device->rx_buffer[0] == 'P' && device->rx_buffer[1] == 'D') {
    return BLISP_ERR_PENDING;  // TODO: This might be rather positive return
                               // number?
  } else if (device->rx_buffer[0] == 'F' && device->rx_buffer[1] == 'L') {
    sp_blocking_read(serial_port, &device->rx_buffer[2], 2, 100);
    device->error_code = (device->rx_buffer[3] << 8) | (device->rx_buffer[2]);
    blisp_dlog("Chip returned error: %d", device->error_code);
    return BLISP_ERR_CHIP_ERR;
  }
  blisp_dlog("Failed to receive any response (err: %d, %d - %d)", ret,
             device->rx_buffer[0], device->rx_buffer[1]);
  return BLISP_ERR_NO_RESPONSE;
}

blisp_return_t blisp_device_handshake(struct blisp_device* device,
                                      bool in_ef_loader) {
  int ret;
  bool ok = false;
  uint8_t handshake_buffer[600];
  struct sp_port* serial_port = device->serial_port;

  if (!in_ef_loader && !device->is_usb) {
    sp_set_rts(serial_port, SP_RTS_ON);
    sp_set_dtr(serial_port, SP_DTR_ON);
    sleep_ms(50);
    sp_set_dtr(serial_port, SP_DTR_OFF);
    sleep_ms(100);
    sp_set_rts(serial_port, SP_RTS_OFF);
    sleep_ms(50);  // Wait a bit so BootROM can init
  }

  uint32_t bytes_count = device->chip->handshake_byte_multiplier *
                         (float)device->current_baud_rate / 10.0f;
  if (bytes_count > 600)
    bytes_count = 600;
  memset(handshake_buffer, 'U', bytes_count);

  for (uint8_t i = 0; i < 5; i++) {
    if (!in_ef_loader) {
      if (device->is_usb) {
        sp_blocking_write(serial_port, "BOUFFALOLAB5555RESET\0\0", 22, 100);
        drain(serial_port);
      }
    }
    ret = sp_blocking_write(serial_port, handshake_buffer, bytes_count, 500);
    // not sure about Apple part, but FreeBSD needs it
    drain(serial_port);
    if (ret < 0) {
      blisp_dlog("Handshake write failed, ret %d", ret);
      return BLISP_ERR_API_ERROR;
    }

    if (!in_ef_loader && !device->is_usb) {
      sp_drain(serial_port);                // Wait for write to send all data
      sp_flush(serial_port, SP_BUF_INPUT);  // Flush garbage out of RX
    }

    if (device->chip->type == BLISP_CHIP_BL808) {
      sleep_ms(300);
      const static uint8_t second_handshake[] = { 0x50, 0x00, 0x08, 0x00, 0x38, 0xF0, 0x00, 0x20, 0x00, 0x00, 0x00, 0x18 };
      ret = sp_blocking_write(serial_port, second_handshake, sizeof(second_handshake), 300);
      if (ret < 0) {
        blisp_dlog("Second handshake write failed, ret %d", ret);
        return BLISP_ERR_API_ERROR;
      }
    }

    ret = sp_blocking_read(serial_port, device->rx_buffer, 20, 50);
    if (ret >= 2) {
      for (uint8_t j = 0; j < (ret - 1); j++) {
        if (device->rx_buffer[j] == 'O' && device->rx_buffer[j + 1] == 'K') {
          ok = true;
        }
      }
    }

    if (!ok) {
      blisp_dlog("Received incorrect handshake response from chip.");
      blisp_dlog_no_nl("Could not find 0x%02X 0x%02X ('O', 'K') in: ", 'O', 'K');
      if (ret) {
        for (uint8_t j=0; j <= ret; j++) {
          blisp_dlog_no_nl("0x%02X ", device->rx_buffer[j]);
        }
      }
      blisp_dlog("");
      return BLISP_ERR_NO_RESPONSE;
    }

    return BLISP_OK;
  }
}

blisp_return_t blisp_device_get_boot_info(struct blisp_device* device,
                                          struct blisp_boot_info* boot_info) {
  blisp_return_t ret;

  ret = blisp_send_command(device, 0x10, NULL, 0, false);
  if (ret < 0)
    return ret;

  ret = blisp_receive_response(device, true);
  if (ret < 0)
    return ret;

  // TODO: Endianess; this may break on big endian machines
  memcpy(boot_info->boot_rom_version, &device->rx_buffer[0], 4);

  if (device->chip->type == BLISP_CHIP_BL70X) {
    memcpy(boot_info->chip_id, &device->rx_buffer[16], 8);
  } else {
    memcpy(boot_info->chip_id, &device->rx_buffer[12], 6);
  }

  return BLISP_OK;
}

// TODO: Use struct instead of uint8_t*
blisp_return_t blisp_device_load_boot_header(struct blisp_device* device,
                                             uint8_t* boot_header) {
  blisp_return_t ret;
  ret = blisp_send_command(device, 0x11, boot_header, 176, false);
  if (ret < 0)
    return ret;
  ret = blisp_receive_response(device, false);
  if (ret < 0)
    return ret;

  return BLISP_OK;
}

blisp_return_t blisp_device_load_segment_header(
    struct blisp_device* device,
    struct blisp_segment_header* segment_header) {
  blisp_return_t ret;
  ret = blisp_send_command(device, 0x17, segment_header, 16, false);
  if (ret < 0)
    return ret;
  ret = blisp_receive_response(device, true);  // TODO: Handle response
  if (ret < 0)
    return ret;

  return BLISP_OK;
}

blisp_return_t blisp_device_load_segment_data(struct blisp_device* device,
                                              uint8_t* segment_data,
                                              uint32_t segment_data_length) {
  blisp_return_t ret;
  ret = blisp_send_command(device, 0x18, segment_data, segment_data_length,
                           false);
  if (ret < 0)
    return ret;
  ret = blisp_receive_response(device, false);
  if (ret < 0)
    return ret;

  return BLISP_OK;
}

blisp_return_t blisp_device_check_image(struct blisp_device* device) {
  blisp_return_t ret;
  ret = blisp_send_command(device, 0x19, NULL, 0, false);
  if (ret < 0)
    return ret;
  ret = blisp_receive_response(device, false);
  if (ret < 0)
    return ret;

  return BLISP_OK;
}

blisp_return_t blisp_device_write_memory(struct blisp_device* device,
                                         uint32_t address,
                                         uint32_t value,
                                         bool wait_for_res) {
  blisp_return_t ret;
  uint8_t payload[8];
  *(uint32_t*)(payload) = address;
  *(uint32_t*)(payload + 4) = value;  // TODO: Endianness
  ret = blisp_send_command(device, 0x50, payload, 8, true);
  if (ret < 0)
    return ret;
  if (wait_for_res) {
    ret = blisp_receive_response(device, false);
    if (ret < 0)
      return ret;
  }

  return BLISP_OK;
}

blisp_return_t blisp_device_run_image(struct blisp_device* device) {
  blisp_return_t ret;

  if (device->chip->type == BLISP_CHIP_BL70X) {  // ERRATA
    ret = blisp_device_write_memory(device, 0x4000F100, 0x4E424845, true);
    if (ret < 0)
      return ret;
    ret = blisp_device_write_memory(device, 0x4000F104, 0x22010000, true);
    if (ret < 0)
      return ret;
    //        ret = blisp_device_write_memory(device, 0x40000018, 0x00000000);
    //        if (ret < 0) return ret;
    ret = blisp_device_write_memory(device, 0x40000018, 0x00000002, false);
    if (ret < 0)
      return ret;
    return BLISP_OK;
  }

  ret = blisp_send_command(device, 0x1A, NULL, 0, false);
  if (ret < 0)
    return ret;
  ret = blisp_receive_response(device, false);
  if (ret < 0)
    return ret;

  return BLISP_OK;
}

blisp_return_t blisp_device_flash_erase(struct blisp_device* device,
                                        uint32_t start_address,
                                        uint32_t end_address) {
  uint8_t payload[8];
  *(uint32_t*)(payload + 0) = start_address;
  *(uint32_t*)(payload + 4) = end_address;

  blisp_return_t ret = blisp_send_command(device, 0x30, payload, 8, true);
  if (ret != BLISP_OK)
    return ret;
  do {
    ret = blisp_receive_response(device, false);
  } while (ret == BLISP_ERR_PENDING);

  return ret;
}

blisp_return_t blisp_device_chip_erase(struct blisp_device* device) {
  blisp_return_t ret = blisp_send_command(device, 0x3C, NULL, 0, true);
  if (ret != BLISP_OK)
    return ret;
  do {
    ret = blisp_receive_response(device, false);
  } while (ret == BLISP_ERR_PENDING);

  return ret;
}

blisp_return_t blisp_device_flash_write(struct blisp_device* device,
                                        uint32_t start_address,
                                        uint8_t* payload,
                                        uint32_t payload_size) {
  // TODO: Add max payload size (8184?)
  // TODO: Don't use malloc + add check

  uint8_t* buffer = malloc(4 + payload_size);
  *((uint32_t*)(buffer)) = start_address;
  memcpy(buffer + 4, payload, payload_size);
  blisp_return_t ret =
      blisp_send_command(device, 0x31, buffer, payload_size + 4, true);
  if (ret < 0)
    goto exit1;
  ret = blisp_receive_response(device, false);
exit1:
  free(buffer);
  return ret;
}

blisp_return_t blisp_device_program_check(struct blisp_device* device) {
  int ret = blisp_send_command(device, 0x3A, NULL, 0, true);
  if (ret < 0)
    return ret;
  ret = blisp_receive_response(device, false);
  if (ret < 0)
    return ret;

  return BLISP_OK;
}

blisp_return_t blisp_device_reset(struct blisp_device* device) {
  blisp_return_t ret = blisp_send_command(device, 0x21, NULL, 0, true);
  if (ret < 0)
    return ret;
  ret = blisp_receive_response(device, false);
  if (ret < 0)
    return ret;

  return BLISP_OK;
}

void blisp_device_close(struct blisp_device* device) {
  struct sp_port* serial_port = device->serial_port;
  sp_close(serial_port);
}

blisp_return_t bl808_load_clock_para(struct blisp_device* device,
                                     bool irq_en, uint32_t baudrate) {
  // XXX: this may be a good place to increase the baudrate for subsequent comms
  const uint32_t clock_para_size = sizeof(struct bl808_boot_clk_cfg_t);
  const uint32_t payload_size = 8 + clock_para_size;
  uint8_t payload[payload_size] = {};

  uint32_t irq_enable = irq_en ? 1 : 0;
  memcpy(&payload[0], &irq_enable, 4);
  memcpy(&payload[4], &baudrate, 4);
  memcpy(&payload[8], &bl808_header.clk_cfg, clock_para_size);

  blisp_return_t ret = blisp_send_command(device, 0x22, payload, payload_size, true);
  if (ret < 0)
    return ret;
  ret = blisp_receive_response(device, false);
  if (ret < 0)
    return ret;

  return BLISP_OK;
}

blisp_return_t bl808_load_flash_para(struct blisp_device* device) {
  // TODO: I don't understand why these parameters are the way they are,
  //       but at least they are labeled. Also, flash_io_mode and flash_clk_delay
  //       seem to be duplicated in the main spi_flash_cfg_t struct?
  const uint8_t flash_pin = 0x4;
  const uint8_t flash_clk_cfg = 0x41;
  const uint8_t flash_io_mode = 0x01;
  const uint8_t flash_clk_delay = 0;
  
  // Yes, these values are (slightly) different to the ones in blisp_chip_bl808.c
  // These values were obtained by observing the raw bytes sent by Bouffalo's
  // own flashing software. So for whatever reason, the flash configuration needs
  // to be different when flashing the chip vs. when the chip is running normally.
  const static struct bl808_spi_flash_cfg_t cfg = {
    .ioMode = 0x04,
    .cReadSupport = 0x01,
    .clkDelay = 0,
    .clkInvert = 0,
    .resetEnCmd = 0x66,
    .resetCmd = 0x99,
    .resetCreadCmd = 0xff,
    .resetCreadCmdSize = 0x03,
    .jedecIdCmd = 0x9f,
    .jedecIdCmdDmyClk = 0,
    .enter32BitsAddrCmd = 0xb7,
    .exit32BitsAddrCmd = 0xe9,
    .sectorSize = 0x04,
    .mid = 0xef,
    .pageSize = 0x100,
    .chipEraseCmd = 0xc7,
    .sectorEraseCmd = 0x20,
    .blk32EraseCmd = 0x52,
    .blk64EraseCmd = 0xd8,
    .writeEnableCmd = 0x06,
    .pageProgramCmd = 0x02,
    .qpageProgramCmd = 0x32,
    .qppAddrMode = 0,
    .fastReadCmd = 0x0b,
    .frDmyClk = 0x01,
    .qpiFastReadCmd = 0x0b,
    .qpiFrDmyClk = 0x01,
    .fastReadDoCmd = 0x3b,
    .frDoDmyClk = 0x01,
    .fastReadDioCmd = 0xbb,
    .frDioDmyClk = 0,
    .fastReadQoCmd = 0x6b,
    .frQoDmyClk = 0x01,
    .fastReadQioCmd = 0xeb,
    .frQioDmyClk = 0x02,
    .qpiFastReadQioCmd = 0xeb,
    .qpiFrQioDmyClk = 0x02,
    .qpiPageProgramCmd = 0x02,
    .writeVregEnableCmd = 0x50,
    .wrEnableIndex = 0,
    .qeIndex = 0x01,
    .busyIndex = 0,
    .wrEnableBit = 0x01,
    .qeBit = 0x01,
    .busyBit = 0,
    .wrEnableWriteRegLen = 0x02,
    .wrEnableReadRegLen = 0x01,
    .qeWriteRegLen = 0x01,
    .qeReadRegLen = 0x01,
    .releasePowerDown = 0xab,
    .busyReadRegLen = 0x01,
    .readRegCmd[0] = 0x05,
    .readRegCmd[1] = 0x35,
    .readRegCmd[2] = 0,
    .readRegCmd[3] = 0,
    .writeRegCmd[0] = 0x01,
    .writeRegCmd[1] = 0x31,
    .writeRegCmd[2] = 0,
    .writeRegCmd[3] = 0,
    .enterQpi = 0x38,
    .exitQpi = 0xff,
    .cReadMode = 0xa0,
    .cRExit = 0xff,
    .burstWrapCmd = 0x77,
    .burstWrapCmdDmyClk = 0x03,
    .burstWrapDataMode = 0x02,
    .burstWrapData = 0x40,
    .deBurstWrapCmd = 0x77,
    .deBurstWrapCmdDmyClk = 0x03,
    .deBurstWrapDataMode = 0x02,
    .deBurstWrapData = 0xf0,
    .timeEsector = 0x12c,
    .timeE32k = 0x4b0,
    .timeE64k = 0x4b0,
    .timePagePgm = 0x05,
    .timeCe = 0x80e8,
    .pdDelay = 0x03,
    .qeData = 0,
  };
  
  const uint32_t payload_size = 4 + sizeof(struct bl808_spi_flash_cfg_t);
  uint8_t payload[payload_size] = {};
  payload[0] = flash_pin;
  payload[1] = flash_clk_cfg;
  payload[2] = flash_io_mode;
  payload[3] = flash_clk_delay;
  memcpy(&payload[4], &cfg, sizeof(struct bl808_spi_flash_cfg_t));

  blisp_return_t ret = blisp_send_command(device, 0x3b, payload, payload_size, true);
  if (ret < 0)
    return ret;
  ret = blisp_receive_response(device, false);
  if (ret < 0)
    return ret;

  return BLISP_OK;
}
