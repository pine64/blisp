#include "../cmd.h"
#include "argtable3.h"
#include <blisp.h>
#include <string.h>
#include <inttypes.h>
#ifdef __linux__
#include <unistd.h>
#include <linux/limits.h>
#elif defined(_MSC_VER)
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#include <windows.h>
#define PATH_MAX MAX_PATH
#endif

#define REG_EXTENDED 1
#define REG_ICASE    (REG_EXTENDED << 1)

static struct arg_rex* cmd;
static struct arg_file* binary_to_write;
static struct arg_str* port_name, *chip_type;
static struct arg_end* end;
static void* cmd_write_argtable[5];

ssize_t
get_binary_folder(char* buffer, uint32_t buffer_size) {
#ifdef __linux__
    readlink("/proc/self/exe", buffer, buffer_size); // TODO: Error handling
    char* pos = strrchr(buffer, '/');
#else
    GetModuleFileName(NULL, buffer, buffer_size);
    char* pos = strrchr(buffer, '\\');
#endif
    pos[0] = '\0';
    return pos - buffer;
}

void blisp_flash_firmware() {
    // TODO: We support currently only BL70X
    if (chip_type->count == 0 || strcmp(chip_type->sval[0], "bl70x") != 0) {
        fprintf(stderr, "Chip type is invalid.\n");
        return;
    }
    struct blisp_device device;
    uint32_t ret;
    ret = blisp_device_init(&device, &blisp_chip_bl70x);
    if (ret != 0) {
        fprintf(stderr, "Failed to init device.\n");
        return;
    }
    ret = blisp_device_open(&device, port_name->count == 1 ? port_name->sval[0] : NULL);
    if (ret != 0) {
        fprintf(stderr, "Failed to open device.\n");
        return;
    }
    printf("Sending a handshake...");
    ret = blisp_device_handshake(&device, false);
    if (ret != 0) {
        fprintf(stderr, "\nFailed to handshake with device.\n");
        goto exit1;
    }
    printf(" OK\nGetting chip info...");
    struct blisp_boot_info boot_info;
    ret = blisp_device_get_boot_info(&device, &boot_info);
    if (ret != 0) {
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

    printf(" BootROM version %d.%d.%d.%d, ChipID: %02X%02X%02X%02X%02X%02X%02X%02X\n",
           boot_info.boot_rom_version[0],
           boot_info.boot_rom_version[1],
           boot_info.boot_rom_version[2],
           boot_info.boot_rom_version[3],
           boot_info.chip_id[0],
           boot_info.chip_id[1],
           boot_info.chip_id[2],
           boot_info.chip_id[3],
           boot_info.chip_id[4],
           boot_info.chip_id[5],
           boot_info.chip_id[6],
           boot_info.chip_id[7]);

    char exe_path[PATH_MAX];
    char eflash_loader_path[PATH_MAX];
    get_binary_folder(exe_path, PATH_MAX); // TODO: Error handling
    snprintf(eflash_loader_path, PATH_MAX, "%s/data/%s/eflash_loader_32m.bin", exe_path, device.chip->type_str);

    FILE* eflash_loader_file = fopen(eflash_loader_path, "rb"); // TODO: Error handling
    uint8_t eflash_loader_header[176];
    fread(eflash_loader_header, 176, 1, eflash_loader_file); // TODO: Error handling

    printf("Loading eflash_loader...\n");
    ret = blisp_device_load_boot_header(&device, eflash_loader_header);
    if (ret != 0) {
        fprintf(stderr, "Failed to load boot header.\n");
        goto exit1;
    }

    uint32_t sent_data = 0;
    uint32_t buffer_size = 0;
    uint8_t buffer[4092];

    // TODO: Real checking of segments count
    for (uint8_t seg_index = 0; seg_index < 1; seg_index++) {
        struct blisp_segment_header segment_header = {0};
        fread(&segment_header, 16, 1, eflash_loader_file); // TODO: Error handling

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
            blisp_device_load_segment_data(&device, buffer, buffer_size); // TODO: Error handling
            sent_data += buffer_size;
            printf("%" PRIu32 "b / %" PRIu32 "b (%.2f%%)\n", sent_data, segment_header.length,
                   (((float)sent_data / (float)segment_header.length) * 100.0f));
        }
    }

    ret = blisp_device_check_image(&device);
    if (ret != 0) {
        fprintf(stderr, "Failed to check image.\n");
        goto exit1;
    }

    ret = blisp_device_run_image(&device);
    if (ret != 0) {
        fprintf(stderr, "Failed to run image.\n");
        goto exit1;
    }

    printf("Sending a handshake...");
    ret = blisp_device_handshake(&device, false);
    if (ret != 0) {
        fprintf(stderr, "\nFailed to handshake with device.\n");
        goto exit1;
    }
    printf(" OK\n");

eflash_loader:


exit1:
    if (eflash_loader_file != NULL) fclose(eflash_loader_file);
    blisp_device_close(&device);
}

int8_t
cmd_write_args_init() {
    cmd_write_argtable[0] = cmd
        = arg_rex1(NULL, NULL, "write", NULL, REG_ICASE, NULL);
    cmd_write_argtable[1] = chip_type = arg_str1("c", "chip", "<chip_type>", "Chip Type (bl70x)");
    cmd_write_argtable[2] = port_name
        = arg_str0("p", "port", "<port_name>", "Name/Path to the Serial Port (empty for search)");
    cmd_write_argtable[3] = binary_to_write
        = arg_file1(NULL, NULL, "<input>", "Binary to write");
    cmd_write_argtable[4] = end = arg_end(10);

    if (arg_nullcheck(cmd_write_argtable) != 0) {
        fprintf(stderr, "insufficient memory\n");
        return -1;
    }
    return 0;
}

void cmd_write_args_print_glossary() {
    fputs("Usage: blisp", stdout);
    arg_print_syntax(stdout,cmd_write_argtable,"\n");
    puts("Writes firmware to SPI Flash");
    arg_print_glossary(stdout,cmd_write_argtable,"  %-25s %s\n");
}

uint8_t
cmd_write_parse_exec(int argc, char** argv) {
    int errors = arg_parse(argc, argv, cmd_write_argtable);
    if (errors == 0) {
        blisp_flash_firmware(); // TODO: Error code?
        return 1;
    } else if (cmd->count == 1) {
        cmd_write_args_print_glossary();
        return 1;
    }
    return 0;
}

void cmd_write_args_print_syntax() {
    arg_print_syntax(stdout,cmd_write_argtable,"\n");
}

void
cmd_write_free() {
    arg_freetable(cmd_write_argtable,
                  sizeof(cmd_write_argtable) / sizeof(cmd_write_argtable[0]));
}

struct cmd cmd_write
    = { "write", cmd_write_args_init, cmd_write_parse_exec, cmd_write_args_print_syntax, cmd_write_free };