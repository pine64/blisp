//
// Created by ralim on 25/09/22.
//
#include "dfu_file.h"

#define DFU_SUFFIX_LENGTH    16
#define LMDFU_PREFIX_LENGTH  8
#define LPCDFU_PREFIX_LENGTH 16

struct dfu_file {
    /* File name */
    const char* name;
    /* Pointer to file loaded into memory */
    const uint8_t* firmware;
    /* Different sizes */
    struct {
        off_t total;
        off_t firmware;
        int prefix;
        int suffix;
    } size;
    /* From prefix fields */
    uint32_t lmdfu_address;
    /* From prefix fields */
    uint32_t prefix_type;

    /* From DFU suffix fields */
    uint32_t dwCRC;
    uint16_t bcdDFU;
    uint16_t idVendor;
    uint16_t idProduct;
    uint16_t bcdDevice;
};

enum prefix_type {
    ZERO_PREFIX,
    DFUSE_PREFIX,
    LMDFU_PREFIX,
    LPCDFU_UNENCRYPTED_PREFIX
};

struct dfu_file parse_dfu_suffix(const uint8_t* file_contents,
                                 size_t file_contents_length);
ssize_t parse_target(const uint8_t* data, uint8_t* out_ealt,
                     uint8_t** out_data, size_t* out_data_size,
                     size_t* out_data_address);
ssize_t get_file_contents(const char* file_path_on_disk,
                          uint8_t** file_contents);

/* Parse a .dfu file and extract its payload and metadata
 * Returns 0 if file parsed correctly, negative on error
 * Inputs:
 * - File path to read from
 *
 * Outputs:
 * - File payload contents
 * - File payload start address
 *
 * Usage:
 *   uint8_t* payload=NULL;
 *   size_t payload_length=0;
 *   int res = dfu_file_path("test.dfu",&payload,&payload_length);
 *   ...
 *   free(payload);
 */

int
dfu_file_parse(const char* file_path_on_disk, uint8_t** payload,
               size_t* payload_length, size_t* payload_address) {
    uint8_t* dfu_file_contents = NULL;
    ssize_t file_size
        = get_file_contents(file_path_on_disk, &dfu_file_contents);
    if (file_size <= 0 || dfu_file_contents == NULL) {
        return -1;
    }
    // Parse DFU data
    struct dfu_file dfu_info = parse_dfu_suffix(dfu_file_contents, file_size);
    // Check if its for a BL* chip
    //    if (dfu_info.idVendor != 0x28E9) {
    //        free(dfu_file_contents);
    //        return -1;
    //    }
    // Okay we have done validation, walk firmware and extract the blob and the
    // offset
    size_t data_consumed = 0;
    while (data_consumed < dfu_info.size.firmware) {
        uint8_t ealt = 0;
        uint8_t* blob = NULL;
        size_t blob_size = 0;
        size_t blob_address = 0;
        ssize_t res = parse_target(dfu_info.firmware + data_consumed, &ealt,
                                   &blob, &blob_size, &blob_address);
        if (res < 0) {
            break;
        }
        if (ealt == 0 && blob_size > 0) {
            // Firmware slot, lets prep this and return
            *payload = calloc(blob_size, 1);
            *payload_length = blob_size;
            *payload_address = blob_address;
            memcpy(*payload, blob, blob_size);
            free(dfu_file_contents);
            return 1;
        }
        data_consumed += res;
    }

    return 0;
}

// Read next target, output data+size+alt. Returns bytes consumed
ssize_t
parse_target(const uint8_t* data, uint8_t* out_ealt, uint8_t** out_data,
             size_t* out_data_size, size_t* out_data_address) {
    if (data == NULL || out_ealt == NULL || out_data == NULL
        || out_data_size == NULL) {
        return -99;
    }
    if (data[0] != 'T' || data[1] != 'a') {
        return -1;
    }

    *out_ealt = data[6];
    uint8_t* tdata = data + 6 + 1 + 4 + 255;
    uint32_t len_tdata = *((uint32_t*)tdata);
    tdata += 4;
    uint32_t num_images = *((uint32_t*)tdata);
    tdata += 4;
    ssize_t blob_length = 6 + 1 + 4 + 255 + 8 + len_tdata;
    // Now read all the image blobs from this target
    for (int i = 0; i < num_images; i++) {
        uint32_t address = *((uint32_t*)tdata);
        tdata += 4;
        uint32_t len = *((uint32_t*)tdata);
        tdata += 4;
        *out_data = tdata;
        *out_data_size = len;
        *out_data_address = address;
        return blob_length;
        // tdata+=len;
    }
    return blob_length;
}

static int
probe_prefix(struct dfu_file* file) {
    const uint8_t* prefix = file->firmware;
    file->size.prefix = 0;
    if (file->size.total < LMDFU_PREFIX_LENGTH)
        return 1;
    if (prefix[0] == 'D' && prefix[1] == 'f' && prefix[2] == 'u'
        && prefix[3] == 'S' && prefix[4] == 'e') {
        // DfuSe header
        // https://sourceforge.net/p/dfu-util/dfu-util/ci/master/tree/dfuse-pack.py#l110

        file->size.prefix = 11;
        file->prefix_type = DFUSE_PREFIX;
        uint8_t numTargets = prefix[10];
        printf("Number DFU Targets: %d\n", numTargets);
    }
    if ((prefix[0] == 0x01) && (prefix[1] == 0x00)) {
        uint32_t payload_length = (prefix[7] << 24) | (prefix[6] << 16)
                                  | (prefix[5] << 8) | prefix[4];
        uint32_t expected_payload_length = (uint32_t)file->size.total
                                           - LMDFU_PREFIX_LENGTH
                                           - file->size.suffix;
        if (payload_length != expected_payload_length)
            return 1;
        file->prefix_type = LMDFU_PREFIX;
        file->size.prefix = LMDFU_PREFIX_LENGTH;
        file->lmdfu_address = 1024 * ((prefix[3] << 8) | prefix[2]);
    } else if (((prefix[0] & 0x3f) == 0x1a) && ((prefix[1] & 0x3f) == 0x3f)) {
        file->prefix_type = LPCDFU_UNENCRYPTED_PREFIX;
        file->size.prefix = LPCDFU_PREFIX_LENGTH;
    }

    if (file->size.prefix + file->size.suffix > file->size.total)
        return 1;
    return 0;
}

struct dfu_file
parse_dfu_suffix(const uint8_t* file_contents,
                 const size_t file_contents_length) {
    // This is nearly 1:1 based on
    // https://sourceforge.net/p/dfu-util/dfu-util/ci/master/tree/src/dfu_file.c#l368
    struct dfu_file output;
    memset(&output, 0, sizeof(output));
    output.firmware = file_contents;
    output.size.total = (off_t)file_contents_length;
    /* Check for possible DFU file suffix by trying to parse one */

    uint32_t crc = 0xffffffff;
    const uint8_t* dfu_suffix;
    int missing_suffix = 0;
    const char* reason;

    if (file_contents_length < DFU_SUFFIX_LENGTH) {
        reason = "File too short for DFU suffix";
        missing_suffix = 1;
        goto checked;
    }

    dfu_suffix = file_contents + file_contents_length - DFU_SUFFIX_LENGTH;

    if (dfu_suffix[10] != 'D' || dfu_suffix[9] != 'F'
        || dfu_suffix[8] != 'U') {
        reason = "Invalid DFU suffix signature";
        missing_suffix = 1;
        goto checked;
    }
    // Calculate contents CRC32
    for (int i = 0; i < file_contents_length - 4; i++) {
        crc = crc32_byte(crc, file_contents[i]);
    }

    output.dwCRC = (dfu_suffix[15] << 24) + (dfu_suffix[14] << 16)
                   + (dfu_suffix[13] << 8) + dfu_suffix[12];

    if (output.dwCRC != crc) {
        reason = "DFU suffix CRC does not match";
        missing_suffix = 1;
        goto checked;
    }

    /* At this point we believe we have a DFU suffix
       so we require further checks to succeed */

    output.bcdDFU = (dfu_suffix[7] << 8) + dfu_suffix[6];

    output.size.suffix = dfu_suffix[11];

    if (output.size.suffix < DFU_SUFFIX_LENGTH) {
        fprintf(stderr, "Unsupported DFU suffix length %d",
                output.size.suffix);
    }

    if (output.size.suffix > file_contents_length) {
        fprintf(stderr, "Invalid DFU suffix length %d", output.size.suffix);
    }

    output.idVendor = (dfu_suffix[5] << 8) + dfu_suffix[4];
    output.idProduct = (dfu_suffix[3] << 8) + dfu_suffix[2];
    output.bcdDevice = (dfu_suffix[1] << 8) + dfu_suffix[0];

checked:
    const int res = probe_prefix(&output);

    if (output.size.prefix) {
        const uint8_t* data = file_contents;
        if (output.prefix_type == DFUSE_PREFIX) {
        } else if (output.prefix_type == LMDFU_PREFIX) {
            printf("Possible TI Stellaris DFU prefix with "
                   "the following properties\n"
                   "Address:        0x%08x\n"
                   "Payload length: %d\n",
                   output.lmdfu_address,
                   data[4] | (data[5] << 8) | (data[6] << 16)
                       | (data[7] << 24));
        } else if (output.prefix_type == LPCDFU_UNENCRYPTED_PREFIX) {
            printf("Possible unencrypted NXP LPC DFU prefix with "
                   "the following properties\n"
                   "Payload length: %d kiByte\n",
                   data[2] >> 1 | (data[3] << 7));
        } else {
            fprintf(stderr, "Unknown DFU prefix type");
        }
        output.firmware
            = output.firmware + output.size.prefix; // shift past prefix
    }
    output.size.firmware
        = output.size.total - (output.size.suffix + output.size.prefix);
    return output;
}

// Returns file size _or_ negative on error
ssize_t
get_file_contents(const char* file_path_on_disk, uint8_t** file_contents) {
    size_t read_count;
    size_t file_size = 0;
    size_t read_total = 0;

    FILE* f;

    if (file_contents == NULL) {
        return -99;
    }

    f = fopen(file_path_on_disk, "rb");
    if (f <= 0) {
        fprintf(stderr, "Could not open file %s for reading",
                file_path_on_disk);
        return -1;
    }

    fseek(f, 0, SEEK_END);
    file_size = ftell(f);
    fseek(f, 0, SEEK_SET);
    *file_contents = calloc(file_size, sizeof(uint8_t));

    while (read_total < file_size) {
        size_t to_read = file_size - read_total;
        /* read() limit on Linux, slightly below MAX_INT on Windows */
        if (to_read > 0x7ffff000)
            to_read = 0x7ffff000;
        read_count = fread((*file_contents) + read_total, 1, to_read, f);
        if (read_count == 0)
            break;
        // If error and not end of file, break
        if (read_count == -1 && !feof(f))
            break;
        read_total += read_count;
    }
    if (read_total != file_size) {
        fprintf(stderr, "Could only read %lld of %lld bytes from %s",
                (long long)read_total, (long long)file_size,
                file_path_on_disk);
        return -1;
    }
    fclose(f);
    return (ssize_t)file_size;
}