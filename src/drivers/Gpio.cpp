// GPIO
#include "../include/Gpio.h"

#include "../include/Mem.h"

GPIO::GPIO(){};

void GPIO::init() {}
void GPIO::unload(){};

const char* GPIO::getName() { return "GPIO Driver\n"; }

unsigned int GPIO::gpio_call(unsigned int pin_number, unsigned int value,
                             unsigned long base, unsigned int field_size,
                             unsigned int field_max) {
  unsigned int field_mask = (1 << field_size) - 1;

  if (pin_number > field_max) return 0;
  if (value > field_mask) return 0;

  unsigned int num_fields = 32 / field_size;
  unsigned long reg = base + ((pin_number / num_fields) * 4);
  unsigned int shift = (pin_number % num_fields) * field_size;

  unsigned int curval = MMIO::read(reg);
  curval &= ~(field_mask << shift);
  curval |= value << shift;
  MMIO::write(reg, curval);

  return 1;
}

unsigned int GPIO::gpio_set(unsigned int pin_number, unsigned int value) {
  return gpio_call(pin_number, value, MMIO::ADDR::GPSET0, 1, GPIO_MAX_PIN);
}
unsigned int GPIO::gpio_clear(unsigned int pin_number, unsigned int value) {
  return gpio_call(pin_number, value, MMIO::ADDR::GPCLR0, 1, GPIO_MAX_PIN);
}
unsigned int GPIO::gpio_pull(unsigned int pin_number, unsigned int value) {
  return gpio_call(pin_number, value, MMIO::ADDR::GPPUPPDN0, 2, GPIO_MAX_PIN);
}
unsigned int GPIO::gpio_function(unsigned int pin_number, unsigned int value) {
  return gpio_call(pin_number, value, MMIO::ADDR::GPFSEL0, 3, GPIO_MAX_PIN);
}

void GPIO::gpio_useAsAlt3(unsigned int pin_number) {
  gpio_pull(pin_number, Pull_None);
  gpio_function(pin_number, GPIO_FUNCTION_ALT3);
}

void GPIO::gpio_useAsAlt5(unsigned int pin_number) {
  gpio_pull(pin_number, Pull_None);
  gpio_function(pin_number, GPIO_FUNCTION_ALT5);
}

void GPIO::gpio_initOutputPinWithPullNone(unsigned int pin_number) {
  gpio_pull(pin_number, Pull_None);
  gpio_function(pin_number, GPIO_FUNCTION_OUT);
}

void GPIO::gpio_setPinOutputBool(unsigned int pin_number,
                                 unsigned int onOrOff) {
  if (onOrOff) {
    gpio_set(pin_number, 1);
  } else {
    gpio_clear(pin_number, 1);
  }
}
