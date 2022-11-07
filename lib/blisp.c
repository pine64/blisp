#include <blisp.h>
#include <libserialport.h>
#include <stdio.h>
#include <string.h>

#define DEBUG

int32_t blisp_device_init(struct blisp_device* device, struct blisp_chip* chip)
{
    device->chip = chip;
    device->is_usb = false;
    return 0;
}

int32_t blisp_device_open(struct blisp_device* device, const char* port_name)
{
    int ret;
    struct sp_port* serial_port = NULL;

    if (port_name != NULL) {
        ret = sp_get_port_by_name(port_name, &serial_port);
        if (ret != SP_OK) {
            return -1; // TODO: Improve error codes
        }
    } else {
        if (!device->chip->usb_isp_available) {
            return -2; // Can't auto-find device due it doesn't have native USB
        }
        struct sp_port **port_list;
        ret = sp_list_ports(&port_list);
        if (ret != SP_OK) {
            return -1; // TODO: Improve error codes
        }
        for (int i = 0; port_list[i] != NULL; i++) {
            struct sp_port *port = port_list[i];

            int vid, pid;
            sp_get_port_usb_vid_pid(port, &vid, &pid);
            if (vid == 0xFFFF && pid == 0xFFFF) {
                ret = sp_get_port_by_name(sp_get_port_name(port), &serial_port);
                if (ret != SP_OK) {
                    return -1; // TODO: Improve error codes
                }
                break;
            }
        }
        sp_free_port_list(port_list);
        if (serial_port == NULL) {
            return -3; // Device not found
        }
    }

    ret = sp_open(serial_port, SP_MODE_READ_WRITE);
    if (ret != SP_OK) { // TODO: Handle not found
        return -1;
    }
    sp_set_bits(serial_port, 8);
    sp_set_parity(serial_port, SP_PARITY_NONE);
    sp_set_stopbits(serial_port, 1);
    sp_set_flowcontrol(serial_port, SP_FLOWCONTROL_NONE);
    uint32_t vid, pid;
    sp_get_port_usb_vid_pid(serial_port, &vid, &pid);
    device->is_usb = pid == 0xFFFF;
//    if (device->is_usb) {
//        device->current_baud_rate = 2000000;
//    } else {
        device->current_baud_rate = 500000;
//    }
    sp_set_baudrate(serial_port, device->current_baud_rate);
    device->serial_port = serial_port;
    return 0;
}

int32_t blisp_send_command(struct blisp_device* device, uint8_t command, void* payload, uint16_t payload_size, bool add_checksum)
{
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
            checksum += *(uint8_t*)(payload + i);
        }
        device->tx_buffer[1] = checksum & 0xFF;
    }
    if (payload_size != 0) {
        memcpy(&device->tx_buffer[4], payload, payload_size);
    }
    ret = sp_blocking_write(serial_port, device->tx_buffer, 4 + payload_size, 1000);
    if (ret != (4 + payload_size)) {
        return -1;
    }
    return 0;
}

int32_t blisp_receive_response(struct blisp_device* device, bool expect_payload) {
    // TODO: Check checksum
    int ret;
    struct sp_port* serial_port = device->serial_port;
    ret = sp_blocking_read(serial_port, &device->rx_buffer[0], 2, 300);
    if (ret < 2) {
#ifdef DEBUG
        fprintf(stderr, "Failed to receive response. (ret = %d)\n", ret);
#endif
        return -1; // TODO: Terrible
    } else if (device->rx_buffer[0] == 'O' && device->rx_buffer[1] == 'K') {
        if (expect_payload) {
            sp_blocking_read(serial_port, &device->rx_buffer[2], 2, 100);
            uint16_t data_length = (device->rx_buffer[3] << 8) | (device->rx_buffer[2]);
            sp_blocking_read(serial_port, &device->rx_buffer[0], data_length, 100);
            return data_length;
        }
        return 0;
    } else if (device->rx_buffer[0] == 'P' && device->rx_buffer[1] == 'D') {
        return -3; // TODO: Terrible
    } else if (device->rx_buffer[0] == 'F' && device->rx_buffer[1] == 'L') {
        sp_blocking_read(serial_port, &device->rx_buffer[2], 2, 100);
        device->error_code = (device->rx_buffer[3] << 8) | (device->rx_buffer[2]);
        return -4; // Failed
    }
#ifdef DEBUG
    fprintf(stderr, "Receive response failed... (err: %d, %d - %d)\n", ret, device->rx_buffer[0], device->rx_buffer[1]);
#endif
    return -1;
}

int32_t
blisp_device_handshake(struct blisp_device* device, bool in_ef_loader) {
    int ret;
    uint8_t handshake_buffer[600];
    struct sp_port* serial_port = device->serial_port;

    uint32_t bytes_count = 0.003f * (float)device->current_baud_rate / 10.0f; // TODO: 0.003f is only for BL70X!
    if (bytes_count > 600) bytes_count = 600;
    memset(handshake_buffer, 'U', bytes_count);

    for (uint8_t i = 0; i < 5; i++) {
        if (!in_ef_loader) {
            if (device->is_usb) {
                sp_blocking_write(serial_port, "BOUFFALOLAB5555RESET\0\0", 22,
                                  100);
            }
        }

        ret = sp_blocking_write(serial_port, handshake_buffer, bytes_count,
                                100);
        if (ret < 0) {
            return -1;
        }
        ret = sp_blocking_read(serial_port, device->rx_buffer, 2, 100);
        if (ret == 2 && device->rx_buffer[0] == 'O' && device->rx_buffer[1] == 'K') {
            return 0;
        }
    }
    return -4; // didn't received response
}

int32_t blisp_device_get_boot_info(struct blisp_device* device, struct blisp_boot_info* boot_info)
{
    int ret;

    ret = blisp_send_command(device, 0x10, NULL, 0, false);
    if (ret < 0) return ret;

    ret = blisp_receive_response(device, true);
    if (ret < 0) return ret;

    if (device->chip->type == BLISP_CHIP_BL70X) {
        memcpy(boot_info->boot_rom_version, &device->rx_buffer[0], 4); // TODO: Endianess
        memcpy(boot_info->chip_id, &device->rx_buffer[16], 8);
    }
    return 0;
}

// TODO: Use struct instead of uint8_t*
int32_t blisp_device_load_boot_header(struct blisp_device* device, uint8_t* boot_header)
{
    int ret;
    ret = blisp_send_command(device, 0x11, boot_header, 176, false);
    if (ret < 0) return ret;
    ret = blisp_receive_response(device, false);
    if (ret < 0) return ret;

    return 0;
}

int32_t blisp_device_load_segment_header(struct blisp_device* device, struct blisp_segment_header* segment_header)
{
    int ret;
    ret = blisp_send_command(device, 0x17, segment_header, 16, false);
    if (ret < 0) return ret;
    ret = blisp_receive_response(device, true); // TODO: Handle response
    if (ret < 0) return ret;

    return 0;
}

int32_t blisp_device_load_segment_data(struct blisp_device* device, uint8_t* segment_data, uint32_t segment_data_length)
{
    int ret;
    ret = blisp_send_command(device, 0x18, segment_data, segment_data_length, false);
    if (ret < 0) return ret;
    ret = blisp_receive_response(device, true); // TODO: Handle response
    if (ret < 0) return ret;

    return 0;
}

int32_t blisp_device_check_image(struct blisp_device* device)
{
    int ret;
    ret = blisp_send_command(device, 0x19, NULL, 0, false);
    if (ret < 0) return ret;
    ret = blisp_receive_response(device, false);
    if (ret < 0) return ret;

    return 0;
}

int32_t
blisp_device_write_memory(struct blisp_device* device, uint32_t address,
                          uint32_t value, bool wait_for_res) {
    int ret;
    uint8_t payload[8];
    *(uint32_t*)(payload) = address;
    *(uint32_t*)(payload + 4) = value; // TODO: Endianness
    ret = blisp_send_command(device, 0x50, payload, 8, true);
    if (ret < 0) return ret;
    if (wait_for_res) {
        ret = blisp_receive_response(device, false);
        if (ret < 0) return ret;
    }

    return 0;
}

int32_t blisp_device_run_image(struct blisp_device* device)
{
    int ret;

    if (device->chip->type == BLISP_CHIP_BL70X) { // ERRATA
        ret = blisp_device_write_memory(device, 0x4000F100, 0x4E424845, true);
        if (ret < 0) return ret;
        ret = blisp_device_write_memory(device, 0x4000F104, 0x22010000, true);
        if (ret < 0) return ret;
//        ret = blisp_device_write_memory(device, 0x40000018, 0x00000000);
//        if (ret < 0) return ret;
        ret = blisp_device_write_memory(device, 0x40000018, 0x00000002, false);
        if (ret < 0) return ret;
        return 0;
    }

    ret = blisp_send_command(device, 0x1A, NULL, 0, false);
    if (ret < 0) return ret;
    ret = blisp_receive_response(device, false);
    if (ret < 0) return ret;

    return 0;
}

void blisp_device_close(struct blisp_device* device)
{
    struct sp_port* serial_port = device->serial_port;
    sp_close(serial_port);
}