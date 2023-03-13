#include "../include/RNG.h"
#define RNG_WARMUP_COUNT 0x0100

RNG::RNG(){};

void RNG::init() {
  MMIO::write(ARM_HW_RNG_STATUS, RNG_WARMUP_COUNT);
  MMIO::write(ARM_HW_RNG_CTRL, ARM_HW_RNG_CTRL_EN);
};
const char* RNG::getName() { return "RNG 200"; };
void RNG::unload(){};
uint32_t RNG::getNumber() {
  uint32_t status;
  do {
    status = MMIO::read(ARM_HW_RNG_STATUS);
    // just wait
  } while ((status >> 24) == 0);
  uint32_t nResult = MMIO::read(ARM_HW_RNG_DATA);
  return nResult;
};
