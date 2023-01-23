#include <stdint.h>

#include "Gpio.h"

/* #include <common.h>
#include <io.h>

#include "peripherals/base.h" */

#define EMMC_DEBUG 0

typedef volatile uint32_t reg32;

#define BSWAP32(x)                                      \
  (((x << 24) & 0xff000000) | ((x << 8) & 0x00ff0000) | \
   ((x >> 8) & 0x0000ff00) | ((x >> 24) & 0x000000ff))

typedef struct {
  uint8_t resp_a : 1;
  uint8_t block_count : 1;
  uint8_t auto_command : 2;
  uint8_t direction : 1;
  uint8_t multiblock : 1;
  uint16_t resp_b : 10;
  uint8_t response_type : 2;
  uint8_t res0 : 1;
  uint8_t crc_enable : 1;
  uint8_t idx_enable : 1;
  uint8_t is_data : 1;
  uint8_t type : 2;
  uint8_t index : 6;
  uint8_t res1 : 2;
} emmc_cmd;

#define RES_CMD \
  { 1, 1, 3, 1, 1, 0xF, 3, 1, 1, 1, 1, 3, 0xF, 3 }

typedef enum { RTNone, RT136, RT48, RT48Busy } cmd_resp_type;

typedef enum {
  CTGoIdle = 0,
  CTSendCide = 2,
  CTSendRelativeAddr = 3,
  CTIOSetOpCond = 5,
  CTSelectCard = 7,
  CTSendIfCond = 8,
  CTSetBlockLen = 16,
  CTReadBlock = 17,
  CTReadMultiple = 18,
  CTWriteBlock = 24,
  CTWriteMultiple = 25,
  CTOcrCheck = 41,
  CTSendSCR = 51,
  CTApp = 55
} cmd_type;

typedef struct {
  uint32_t scr[2];
  uint32_t bus_widths;
  uint32_t version;
} scr_register;

typedef enum {
  SDECommandTimeout,
  SDECommandCrc,
  SDECommandEndBit,
  SDECommandIndex,
  SDEDataTimeout,
  SDEDataCrc,
  SDEDataEndBit,
  SDECurrentLimit,
  SDEAutoCmd12,
  SDEADma,
  SDETuning,
  SDERsvd
} sd_error;

typedef struct {
  bool last_success;
  uint32_t transfer_blocks;
  emmc_cmd last_command;
  reg32 last_command_value;
  uint32_t block_size;
  uint32_t last_response[4];
  bool sdhc;
  uint16_t ocr;
  uint32_t rca;
  uint64_t offset;
  void *buffer;
  uint32_t base_clock;
  uint32_t last_error;
  uint32_t last_interrupt;
  scr_register scr;
} emmc_device;

typedef struct {
  reg32 arg2;
  reg32 block_size_count;
  reg32 arg1;
  reg32 cmd_xfer_mode;
  reg32 response[4];
  reg32 data;
  reg32 status;
  reg32 control[2];
  reg32 int_flags;
  reg32 int_mask;
  reg32 int_enable;
  reg32 control2;
  reg32 cap1;
  reg32 cap2;
  reg32 res0[2];
  reg32 force_int;
  reg32 res1[7];
  reg32 boot_timeout;
  reg32 debug_config;
  reg32 res2[2];
  reg32 ext_fifo_config;
  reg32 ext_fifo_enable;
  reg32 tune_step;
  reg32 tune_SDR;
  reg32 tune_DDR;
  reg32 res3[23];
  reg32 spi_int_support;
  reg32 res4[2];
  reg32 slot_int_status;
} emmc_regs;

#define TO_REG(p) *((reg32 *)p)

// SD Clock Frequencies (in Hz)
#define SD_CLOCK_ID 400000
#define SD_CLOCK_NORMAL 25000000
#define SD_CLOCK_HIGH 50000000
#define SD_CLOCK_100 100000000
#define SD_CLOCK_208 208000000
#define SD_COMMAND_COMPLETE 1
#define SD_TRANSFER_COMPLETE (1 << 1)
#define SD_BLOCK_GAP_EVENT (1 << 2)
#define SD_DMA_INTERRUPT (1 << 3)
#define SD_BUFFER_WRITE_READY (1 << 4)
#define SD_BUFFER_READ_READY (1 << 5)
#define SD_CARD_INSERTION (1 << 6)
#define SD_CARD_REMOVAL (1 << 7)
#define SD_CARD_INTERRUPT (1 << 8)

#define EMMC_BASE (0xffff000000000000 + 0xFE000000 + 0x00340000)

#define EMMC ((emmc_regs *)EMMC_BASE)

int emmc_read(uint8_t *buffer, uint32_t size);
int emmc_write(uint8_t *buffer, uint32_t size);

void emmc_seek(uint64_t offset);
bool emmc_init(GPIO *gpio);

#define EMMC_CTRL1_RESET_DATA (1 << 26)
#define EMMC_CTRL1_RESET_CMD (1 << 25)
#define EMMC_CTRL1_RESET_HOST (1 << 24)
#define EMMC_CTRL1_RESET_ALL \
  (EMMC_CTRL1_RESET_DATA | EMMC_CTRL1_RESET_CMD | EMMC_CTRL1_RESET_HOST)

#define EMMC_CTRL1_CLK_GENSEL (1 << 5)
#define EMMC_CTRL1_CLK_ENABLE (1 << 2)
#define EMMC_CTRL1_CLK_STABLE (1 << 1)
#define EMMC_CTRL1_CLK_INT_EN (1 << 0)

#define EMMC_CTRL0_ALT_BOOT_EN (1 << 22)
#define EMMC_CTRL0_BOOT_EN (1 << 21)
#define EMMC_CTRL0_SPI_MODE (1 << 20)

#define EMMC_STATUS_DAT_INHIBIT (1 << 1)
#define EMMC_STATUS_CMD_INHIBIT (1 << 0)
