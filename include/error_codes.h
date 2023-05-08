#ifndef BLISP_S_RC_ERROR_CODES_H_
#define BLISP_S_RC_ERROR_CODES_H_

typedef enum {
  BLISP_OK = 0,
  // All error states must be <0.
  // Generic error return; for when we are unsure what failed
  BLISP_ERR_UNKNOWN = -1,
  // Device did not respond, if serial link, could be that its not in bootloader
  BLISP_ERR_NO_RESPONSE = -2,
  // Failed to open a device, likely libusb or permissions
  BLISP_ERR_DEVICE_NOT_FOUND = -3,
  BLISP_ERR_CANT_OPEN_DEVICE = -4,
  // Can't auto-find device due it doesn't have native USB
  BLISP_ERR_NO_AUTO_FIND_AVAILABLE = -5,
  BLISP_ERR_PENDING = -6,
  BLISP_ERR_CHIP_ERR = -7,
  BLISP_ERR_INVALID_CHIP_TYPE = -8,
  BLISP_ERR_OUT_OF_MEMORY = -9,
  BLISP_ERR_INVALID_COMMAND = -10,
  BLISP_ERR_CANT_OPEN_FILE=-11,

} blisp_return_t;
#endif