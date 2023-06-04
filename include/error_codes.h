#ifndef BLISP_S_RC_ERROR_CODES_H_
#define BLISP_S_RC_ERROR_CODES_H_

typedef enum {
  BLISP_OK = 0,
  // All error states must be <0.
  // Generic error return; for when we are unsure what failed
  BLISP_ERR_UNKNOWN = -1,
  // Device did not respond, if serial link, could be that its not in boot
  // loader
  BLISP_ERR_NO_RESPONSE = -2,
  // Failed to open a device, likely libusb or permissions
  BLISP_ERR_DEVICE_NOT_FOUND = -3,  // We could not find a device
  BLISP_ERR_CANT_OPEN_DEVICE =
      -4,  // Couldn't open device; could it be permissions or its in use?
  // Can't auto-find device due it doesn't have native USB
  BLISP_ERR_NO_AUTO_FIND_AVAILABLE = -5,
  BLISP_ERR_PENDING = -6,  // Internal error for device is busy and to come back
  BLISP_ERR_CHIP_ERR = -7,           // Chip returned an error to us
  BLISP_ERR_INVALID_CHIP_TYPE = -8,  // unsupported chip type provided
  BLISP_ERR_OUT_OF_MEMORY =
      -9,  // System could not allocate enough ram (highly unlikely)
  BLISP_ERR_INVALID_COMMAND = -10,  // Invalid user command provided
  BLISP_ERR_CANT_OPEN_FILE = -11,   // Cant open the firmware file to flash
  BLISP_ERR_NOT_IMPLEMENTED = -12,  // Non implemented function called
  BLISP_ERR_API_ERROR = -13,        // Errors outside our control from api's we
                              // integrate (Generally serial port/OS related)

} blisp_return_t;
#endif