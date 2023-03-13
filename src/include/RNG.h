#ifndef RNG_H
#define RNG_H
#include <stddef.h>

#include "Driver.h"
#include "Mem.h"

//
// Hardware Random Number Generator
//

#define ARM_HW_RNG_BASE (MMIO::PERIPHERAL_BASE + 0x104000)

#define ARM_HW_RNG_CTRL (ARM_HW_RNG_BASE + 0x00)
#define ARM_HW_RNG_CTRL_EN 0x01
#define ARM_HW_RNG_STATUS (ARM_HW_RNG_BASE + 0x04)
#define ARM_HW_RNG_DATA (ARM_HW_RNG_BASE + 0x08)

/*

Constants from Linux source

*/

#define RNG_BASE_ADDRESS (MMIO::PERIPHERAL_BASE + 0x104000)

#define RNG_CTRL_OFFSET 0x00
#define RNG_CTRL_RNG_RBGEN_MASK 0x00001FFF
#define RNG_CTRL_RNG_RBGEN_ENABLE 0x00000001

#define RNG_SOFT_RESET_OFFSET 0x04
#define RNG_SOFT_RESET 0x00000001

#define RBG_SOFT_RESET_OFFSET 0x08
#define RBG_SOFT_RESET 0x00000001

#define RNG_INT_STATUS_OFFSET 0x18
#define RNG_INT_STATUS_MASTER_FAIL_LOCKOUT_IRQ_MASK 0x80000000
#define RNG_INT_STATUS_STARTUP_TRANSITIONS_MET_IRQ_MASK 0x00020000
#define RNG_INT_STATUS_NIST_FAIL_IRQ_MASK 0x00000020
#define RNG_INT_STATUS_TOTAL_BITS_COUNT_IRQ_MASK 0x00000001

#define RNG_FIFO_DATA_OFFSET 0x20

#define RNG_FIFO_COUNT_OFFSET 0x24
#define RNG_FIFO_COUNT_RNG_FIFO_COUNT_MASK 0x000000FF

class RNG : public Driver {
 public:
  // Driver Interface
  void init();
  const char* getName();
  void unload();

  // RNG 200
  RNG();
  uint32_t getNumber();

 private:
};

#endif