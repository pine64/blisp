/*
 * Some parts of this source code belongs to Bouffalo Labs
 * COPYRIGHT(c) 2020 Bouffalo Lab , License: Apache
 */

#ifndef _LIBBLISP_STRUCT_H
#define _LIBBLISP_STRUCT_H

#include <assert.h>
#include <stdint.h>

#if !defined(static_assert) && (defined(__GNUC__) || defined(__clang__)) \
    && defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L \
    && __STDC_VERSION__ <= 201710L
#define static_assert _Static_assert
#endif

#pragma pack(push, 1)

typedef struct {
  uint8_t ioMode; /*!< Serail flash interface mode,bit0-3:IF mode,bit4:unwrap */
  uint8_t cReadSupport;  /*!< Support continuous read mode,bit0:continuous read
                            mode support,bit1:read mode cfg */
  uint8_t clkDelay;      /*!< SPI clock delay,bit0-3:delay,bit4-6:pad delay */
  uint8_t clkInvert;     /*!< SPI clock phase invert,bit0:clck invert,bit1:rx
                            invert,bit2-4:pad delay,bit5-7:pad delay */
  uint8_t resetEnCmd;    /*!< Flash enable reset command */
  uint8_t resetCmd;      /*!< Flash reset command */
  uint8_t resetCreadCmd; /*!< Flash reset continuous read command */
  uint8_t resetCreadCmdSize;   /*!< Flash reset continuous read command size */
  uint8_t jedecIdCmd;          /*!< JEDEC ID command */
  uint8_t jedecIdCmdDmyClk;    /*!< JEDEC ID command dummy clock */
  uint8_t qpiJedecIdCmd;       /*!< QPI JEDEC ID comamnd */
  uint8_t qpiJedecIdCmdDmyClk; /*!< QPI JEDEC ID command dummy clock */
  uint8_t sectorSize;          /*!< *1024bytes */
  uint8_t mid;                 /*!< Manufacturer ID */
  uint16_t pageSize;           /*!< Page size */
  uint8_t chipEraseCmd;        /*!< Chip erase cmd */
  uint8_t sectorEraseCmd;      /*!< Sector erase command */
  uint8_t blk32EraseCmd; /*!< Block 32K erase command,some Micron not support */
  uint8_t blk64EraseCmd; /*!< Block 64K erase command */
  uint8_t writeEnableCmd;      /*!< Need before every erase or program */
  uint8_t pageProgramCmd;      /*!< Page program cmd */
  uint8_t qpageProgramCmd;     /*!< QIO page program cmd */
  uint8_t qppAddrMode;         /*!< QIO page program address mode */
  uint8_t fastReadCmd;         /*!< Fast read command */
  uint8_t frDmyClk;            /*!< Fast read command dummy clock */
  uint8_t qpiFastReadCmd;      /*!< QPI fast read command */
  uint8_t qpiFrDmyClk;         /*!< QPI fast read command dummy clock */
  uint8_t fastReadDoCmd;       /*!< Fast read dual output command */
  uint8_t frDoDmyClk;          /*!< Fast read dual output command dummy clock */
  uint8_t fastReadDioCmd;      /*!< Fast read dual io comamnd */
  uint8_t frDioDmyClk;         /*!< Fast read dual io command dummy clock */
  uint8_t fastReadQoCmd;       /*!< Fast read quad output comamnd */
  uint8_t frQoDmyClk;          /*!< Fast read quad output comamnd dummy clock */
  uint8_t fastReadQioCmd;      /*!< Fast read quad io comamnd */
  uint8_t frQioDmyClk;         /*!< Fast read quad io comamnd dummy clock */
  uint8_t qpiFastReadQioCmd;   /*!< QPI fast read quad io comamnd */
  uint8_t qpiFrQioDmyClk;      /*!< QPI fast read QIO dummy clock */
  uint8_t qpiPageProgramCmd;   /*!< QPI program command */
  uint8_t writeVregEnableCmd;  /*!< Enable write reg */
  uint8_t wrEnableIndex;       /*!< Write enable register index */
  uint8_t qeIndex;             /*!< Quad mode enable register index */
  uint8_t busyIndex;           /*!< Busy status register index */
  uint8_t wrEnableBit;         /*!< Write enable bit pos */
  uint8_t qeBit;               /*!< Quad enable bit pos */
  uint8_t busyBit;             /*!< Busy status bit pos */
  uint8_t wrEnableWriteRegLen; /*!< Register length of write enable */
  uint8_t wrEnableReadRegLen;  /*!< Register length of write enable status */
  uint8_t qeWriteRegLen;       /*!< Register length of contain quad enable */
  uint8_t qeReadRegLen; /*!< Register length of contain quad enable status */
  uint8_t releasePowerDown;   /*!< Release power down command */
  uint8_t busyReadRegLen;     /*!< Register length of contain busy status */
  uint8_t readRegCmd[4];      /*!< Read register command buffer */
  uint8_t writeRegCmd[4];     /*!< Write register command buffer */
  uint8_t enterQpi;           /*!< Enter qpi command */
  uint8_t exitQpi;            /*!< Exit qpi command */
  uint8_t cReadMode;          /*!< Config data for continuous read mode */
  uint8_t cRExit;             /*!< Config data for exit continuous read mode */
  uint8_t burstWrapCmd;       /*!< Enable burst wrap command */
  uint8_t burstWrapCmdDmyClk; /*!< Enable burst wrap command dummy clock */
  uint8_t burstWrapDataMode;  /*!< Data and address mode for this command */
  uint8_t burstWrapData;      /*!< Data to enable burst wrap */
  uint8_t deBurstWrapCmd;     /*!< Disable burst wrap command */
  uint8_t deBurstWrapCmdDmyClk; /*!< Disable burst wrap command dummy clock */
  uint8_t deBurstWrapDataMode;  /*!< Data and address mode for this command */
  uint8_t deBurstWrapData;      /*!< Data to disable burst wrap */
  uint16_t timeEsector;         /*!< 4K erase time */
  uint16_t timeE32k;            /*!< 32K erase time */
  uint16_t timeE64k;            /*!< 64K erase time */
  uint16_t timePagePgm;         /*!< Page program time */
  uint16_t timeCe;              /*!< Chip erase time in ms */
  uint8_t pdDelay; /*!< Release power down command delay time for wake up */
  uint8_t qeData;  /*!< QE set data */
} SPI_Flash_Cfg_Type;

#define BFLB_BOOTROM_HASH_SIZE 256 / 8

struct boot_flash_cfg_t {
  char magiccode[4]; /*'FCFG'*/
  SPI_Flash_Cfg_Type cfg;
  uint32_t crc32;
};

struct sys_clk_cfg_t {
  uint8_t xtal_type;
  uint8_t pll_clk;
  uint8_t hclk_div;
  uint8_t bclk_div;

  uint8_t flash_clk_type;
  uint8_t flash_clk_div;
  uint8_t rsvd[2];
};

struct boot_clk_cfg_t {
  char magiccode[4]; /*'PCFG'*/
  struct sys_clk_cfg_t cfg;
  uint32_t crc32;
};

struct bfl_boot_header {
  char magiccode[4]; /*'BFXP'*/
  uint32_t revison;
  struct boot_flash_cfg_t flashCfg;
  struct boot_clk_cfg_t clkCfg;
  union {
    struct {
      uint32_t sign : 2;         /* [1: 0]  for sign*/
      uint32_t encrypt_type : 2; /* [3: 2]  for encrypt */
      uint32_t key_sel : 2;      /* [5: 4]  for key sel in boot interface*/
      uint32_t rsvd6_7 : 2;      /* [7: 6]  for encrypt*/
      uint32_t no_segment : 1;   /* [8]     no segment info */
      uint32_t cache_enable : 1; /* [9]     for cache */
      uint32_t
          notload_in_bootrom : 1;     /* [10]    not load this img in bootrom */
      uint32_t aes_region_lock : 1;   /* [11]    aes region lock */
      uint32_t cache_way_disable : 4; /* [15: 12]  cache way disable info*/
      uint32_t crc_ignore : 1;        /* [16]    ignore crc */
      uint32_t hash_ignore : 1;       /* [17]    hash crc */
      uint32_t halt_ap : 1;           /* [18]    halt ap */
      uint32_t rsvd19_31 : 13;        /* [31:19]  rsvd */
    } bval;
    uint32_t wval;
  } bootcfg;

  union {
    uint32_t segment_cnt;
    uint32_t img_length;
  } segment_info;

  uint32_t bootentry; /* entry point of the image*/

  uint32_t flashoffset;

  uint8_t hash[BFLB_BOOTROM_HASH_SIZE]; /*hash of the image*/

  uint32_t rsv1;
  uint32_t rsv2;
  uint32_t crc32;
};

static_assert(sizeof(struct bfl_boot_header) == 176,
              "Bootheader have wrong size");

struct blflash_segment_header {
  uint32_t destaddr;
  uint32_t len;
  uint32_t rsvd;
  uint32_t crc32;
};

static_assert(sizeof(struct blflash_segment_header) == 16,
              "Segment header have wrong size");

#pragma pack(pop)

#endif
