#include "../include/emmc.h"
#include "../include/Core.h"
#include "../include/Console.h"
#include "../include/mailbox.h"
bool wait_reg_mask(reg32 *reg, uint32_t mask, bool set, uint32_t timeout);

uint32_t get_clock_divider(uint32_t base_clock, uint32_t target_rate) {
  uint32_t target_div = 1;

  if (target_rate <= base_clock) {
    target_div = base_clock / target_rate;

    if (base_clock % target_rate) {
      target_div = 0;
    }
  }

  int div = -1;
  for (int fb = 31; fb >= 0; fb--) {
    uint32_t bt = (1 << fb);

    if (target_div & bt) {
      div = fb;
      target_div &= ~(bt);

      if (target_div) {
        div++;
      }

      break;
    }
  }

  if (div == -1) {
    div = 31;
  }

  if (div >= 32) {
    div = 31;
  }

  if (div != 0) {
    div = (1 << (div - 1));
  }

  if (div >= 0x400) {
    div = 0x3FF;
  }

  uint32_t freqSel = div & 0xff;
  uint32_t upper = (div >> 8) & 0x3;
  uint32_t ret = (freqSel << 8) | (upper << 6) | (0 << 5);

  return ret;
}

bool switch_clock_rate(uint32_t base_clock, uint32_t target_rate) {
  uint32_t divider = get_clock_divider(base_clock, target_rate);

  while ((EMMC->status & (EMMC_STATUS_CMD_INHIBIT | EMMC_STATUS_DAT_INHIBIT))) {
    Core::spinms(1);
  }

  uint32_t c1 = EMMC->control[1] & ~EMMC_CTRL1_CLK_ENABLE;

  EMMC->control[1] = c1;

  Core::spinms(3);

  EMMC->control[1] = (c1 | divider) & ~0xFFE0;

  Core::spinms(3);

  EMMC->control[1] = c1 | EMMC_CTRL1_CLK_ENABLE;

  Core::spinms(3);

  return true;
}

bool emmc_setup_clock() {
  EMMC->control2 = 0;

  uint32_t rate = mailbox_clock_rate(CT_EMMC);

  uint32_t n = EMMC->control[1];
  n |= EMMC_CTRL1_CLK_INT_EN;
  n |= get_clock_divider(rate, SD_CLOCK_ID);
  n &= ~(0xf << 16);
  n |= (11 << 16);

  EMMC->control[1] = n;

  if (!wait_reg_mask(&EMMC->control[1], EMMC_CTRL1_CLK_STABLE, true, 2000)) {
    Console::print("EMMC_ERR: SD CLOCK NOT STABLE\n");
    return false;
  }

  Core::spinms(30);

  // enabling the clock
  EMMC->control[1] |= 4;

  Core::spinms(30);

  return true;
}
