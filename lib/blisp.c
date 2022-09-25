#include <blisp.h>
#include <libserialport.h>
#include <stdio.h>
#include <string.h>

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
    if (device->is_usb) {
        device->current_baud_rate = 2000000;
    } else {
        device->current_baud_rate = 500000;
    }
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
    device->tx_buffer[2] = (payload_size >> 8) & 0xFF;
    device->tx_buffer[3] = payload_size & 0xFF;
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
    int ret;
    struct sp_port* serial_port = device->serial_port;
    ret = sp_blocking_read(serial_port, &device->rx_buffer[0], 2, 300);
    if (ret < 2) {
        return -1;
    }

}

int32_t blisp_device_handshake(struct blisp_device* device)
{
    int ret;
    uint8_t handshake_buffer[600];
    struct sp_port* serial_port = device->serial_port;

    if (device->is_usb) {
        sp_blocking_write(serial_port, "BOUFFALOLAB5555RESET\0\0", 22, 100);
    }
    uint32_t bytes_count = 0.003f * (float)device->current_baud_rate / 10.0f; // TODO: 0.003f is only for BL70X!
    if (bytes_count > 600) bytes_count = 600;
    memset(handshake_buffer, 'U', bytes_count);
    ret = sp_blocking_write(serial_port, handshake_buffer, bytes_count, 100);
    if (ret < 0) {
        return -1;
    }
    ret = sp_blocking_read(serial_port, device->rx_buffer, 2, 100);
    if (ret < 2 || device->rx_buffer[0] != 'O' || device->rx_buffer[1] != 'K') {
        return -4; // didn't received response
    }
    return 0;
}

int32_t blisp_device_get_boot_info(struct blisp_device* device, struct blisp_boot_info* boot_info)
{
    int ret;

    ret = blisp_send_command(device, 0x10, NULL, 0, false);
    if (ret < 0) return ret;

    ret = blisp_receive_response(device, true);

    return 0;
}

void blisp_device_close(struct blisp_device* device)
{
    struct sp_port* serial_port = device->serial_port;
    sp_close(serial_port);
}